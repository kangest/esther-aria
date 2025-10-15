#include "RestaurantDataManager.h"
#include "Json.h"
#include "JsonObjectConverter.h"
#include "Engine/World.h"
#include "TimerManager.h"

ARestaurantDataManager::ARestaurantDataManager()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ARestaurantDataManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Initialize API keys from project settings or environment
    // These should be set via Blueprint or configuration
    UE_LOG(LogTemp, Log, TEXT("RestaurantDataManager initialized"));
}

void ARestaurantDataManager::SetAPIKeys(const FString& GooglePlacesKey, const FString& YelpKey)
{
    GooglePlacesAPIKey = GooglePlacesKey;
    YelpAPIKey = YelpKey;
    
    UE_LOG(LogTemp, Log, TEXT("API keys configured"));
}

void ARestaurantDataManager::SearchRestaurants(FVector2D Location, const FSearchFilters& Filters)
{
    CurrentSearchLocation = Location;
    CurrentFilters = Filters;
    
    // Check cache first
    FString CacheKey = GenerateCacheKey(Location, Filters);
    if (IsCacheValid(CacheKey))
    {
        TArray<FRestaurantData> CachedResults = RestaurantCache[CacheKey];
        OnRestaurantsFound.Broadcast(CachedResults);
        return;
    }
    
    // Reset completion flags
    bGooglePlacesComplete = false;
    bYelpComplete = false;
    PendingRequests = 0;
    
    // Start parallel API requests
    if (!GooglePlacesAPIKey.IsEmpty())
    {
        SearchGooglePlaces(Location, Filters);
    }
    else
    {
        bGooglePlacesComplete = true;
    }
    
    if (!YelpAPIKey.IsEmpty())
    {
        SearchYelp(Location, Filters);
    }
    else
    {
        bYelpComplete = true;
    }
    
    // If no API keys are configured, return empty results
    if (GooglePlacesAPIKey.IsEmpty() && YelpAPIKey.IsEmpty())
    {
        HandleAPIError("Configuration", "No API keys configured");
        OnRestaurantsFound.Broadcast(TArray<FRestaurantData>());
    }
}

void ARestaurantDataManager::SearchGooglePlaces(FVector2D Location, const FSearchFilters& Filters)
{
    FString URL = BuildGooglePlacesSearchURL(Location, Filters);
    
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &ARestaurantDataManager::OnGooglePlacesResponse);
    Request->SetURL(URL);
    Request->SetVerb("GET");
    Request->SetHeader("Content-Type", "application/json");
    
    PendingRequests++;
    Request->ProcessRequest();
    
    UE_LOG(LogTemp, Log, TEXT("Google Places request sent: %s"), *URL);
}

void ARestaurantDataManager::SearchYelp(FVector2D Location, const FSearchFilters& Filters)
{
    FString URL = BuildYelpSearchURL(Location, Filters);
    
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &ARestaurantDataManager::OnYelpResponse);
    Request->SetURL(URL);
    Request->SetVerb("GET");
    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("Authorization", FString::Printf(TEXT("Bearer %s"), *YelpAPIKey));
    
    PendingRequests++;
    Request->ProcessRequest();
    
    UE_LOG(LogTemp, Log, TEXT("Yelp request sent: %s"), *URL);
}

FString ARestaurantDataManager::BuildGooglePlacesSearchURL(FVector2D Location, const FSearchFilters& Filters)
{
    FString URL = GooglePlacesBaseURL + "nearbysearch/json?";
    URL += FString::Printf(TEXT("location=%f,%f"), Location.X, Location.Y);
    URL += FString::Printf(TEXT("&radius=%f"), Filters.MaxDistance);
    URL += TEXT("&type=restaurant");
    
    if (Filters.bOpenNow)
    {
        URL += TEXT("&opennow=true");
    }
    
    if (!Filters.CuisineTypes.IsEmpty())
    {
        URL += TEXT("&keyword=") + FString::Join(Filters.CuisineTypes, TEXT("+"));
    }
    
    URL += TEXT("&key=") + GooglePlacesAPIKey;
    
    return URL;
}

FString ARestaurantDataManager::BuildYelpSearchURL(FVector2D Location, const FSearchFilters& Filters)
{
    FString URL = YelpBaseURL + "businesses/search?";
    URL += FString::Printf(TEXT("latitude=%f&longitude=%f"), Location.X, Location.Y);
    URL += FString::Printf(TEXT("&radius=%d"), FMath::RoundToInt(Filters.MaxDistance));
    URL += TEXT("&categories=restaurants");
    URL += TEXT("&limit=50");
    
    if (Filters.bOpenNow)
    {
        URL += TEXT("&open_now=true");
    }
    
    if (!Filters.CuisineTypes.IsEmpty())
    {
        URL += TEXT("&term=") + FString::Join(Filters.CuisineTypes, TEXT("+"));
    }
    
    return URL;
}

void ARestaurantDataManager::OnGooglePlacesResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    PendingRequests--;
    bGooglePlacesComplete = true;
    
    TArray<FRestaurantData> GoogleResults;
    
    if (bWasSuccessful && Response.IsValid())
    {
        FString ResponseBody = Response->GetContentAsString();
        GoogleResults = ParseGooglePlacesResponse(ResponseBody);
        
        UE_LOG(LogTemp, Log, TEXT("Google Places returned %d results"), GoogleResults.Num());
    }
    else
    {
        HandleAPIError("GooglePlaces", "Request failed");
    }
    
    // Store results temporarily
    if (!GoogleResults.IsEmpty())
    {
        for (FRestaurantData& Restaurant : GoogleResults)
        {
            CurrentRestaurants.Add(Restaurant);
        }
    }
    
    // Check if all requests are complete
    CheckRequestsComplete();
}

void ARestaurantDataManager::OnYelpResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    PendingRequests--;
    bYelpComplete = true;
    
    TArray<FRestaurantData> YelpResults;
    
    if (bWasSuccessful && Response.IsValid())
    {
        FString ResponseBody = Response->GetContentAsString();
        YelpResults = ParseYelpResponse(ResponseBody);
        
        UE_LOG(LogTemp, Log, TEXT("Yelp returned %d results"), YelpResults.Num());
    }
    else
    {
        HandleAPIError("Yelp", "Request failed");
    }
    
    // Merge with existing results
    if (!YelpResults.IsEmpty())
    {
        for (const FRestaurantData& YelpRestaurant : YelpResults)
        {
            // Try to find matching restaurant in current results
            bool bFound = false;
            for (FRestaurantData& ExistingRestaurant : CurrentRestaurants)
            {
                if (ExistingRestaurant.Name.Equals(YelpRestaurant.Name, ESearchCase::IgnoreCase) ||
                    FVector2D::Distance(ExistingRestaurant.Location, YelpRestaurant.Location) < 50.0f)
                {
                    MergeRestaurantData(ExistingRestaurant, YelpRestaurant);
                    bFound = true;
                    break;
                }
            }
            
            if (!bFound)
            {
                CurrentRestaurants.Add(YelpRestaurant);
            }
        }
    }
    
    CheckRequestsComplete();
}

void ARestaurantDataManager::CheckRequestsComplete()
{
    if (bGooglePlacesComplete && bYelpComplete)
    {
        // Sort results by relevance
        SortByRelevance(CurrentRestaurants);
        
        // Cache results
        FString CacheKey = GenerateCacheKey(CurrentSearchLocation, CurrentFilters);
        RestaurantCache.Add(CacheKey, CurrentRestaurants);
        CacheTimestamps.Add(CacheKey, FDateTime::Now());
        
        // Broadcast results
        OnRestaurantsFound.Broadcast(CurrentRestaurants);
        
        UE_LOG(LogTemp, Log, TEXT("Search complete. Found %d restaurants"), CurrentRestaurants.Num());
    }
}

TArray<FRestaurantData> ARestaurantDataManager::ParseGooglePlacesResponse(const FString& ResponseBody)
{
    TArray<FRestaurantData> Results;
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
    
    if (FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        const TArray<TSharedPtr<FJsonValue>>* ResultsArray;
        if (JsonObject->TryGetArrayField(TEXT("results"), ResultsArray))
        {
            for (const auto& ResultValue : *ResultsArray)
            {
                TSharedPtr<FJsonObject> ResultObject = ResultValue->AsObject();
                if (ResultObject.IsValid())
                {
                    FRestaurantData Restaurant;
                    
                    // Basic info
                    ResultObject->TryGetStringField(TEXT("name"), Restaurant.Name);
                    ResultObject->TryGetStringField(TEXT("vicinity"), Restaurant.Address);
                    ResultObject->TryGetStringField(TEXT("place_id"), Restaurant.GooglePlaceId);
                    
                    // Location
                    const TSharedPtr<FJsonObject>* GeometryObject;
                    if (ResultObject->TryGetObjectField(TEXT("geometry"), GeometryObject))
                    {
                        const TSharedPtr<FJsonObject>* LocationObject;
                        if ((*GeometryObject)->TryGetObjectField(TEXT("location"), LocationObject))
                        {
                            double Lat, Lng;
                            if ((*LocationObject)->TryGetNumberField(TEXT("lat"), Lat) &&
                                (*LocationObject)->TryGetNumberField(TEXT("lng"), Lng))
                            {
                                Restaurant.Location = FVector2D(Lat, Lng);
                            }
                        }
                    }
                    
                    // Rating
                    double Rating;
                    if (ResultObject->TryGetNumberField(TEXT("rating"), Rating))
                    {
                        Restaurant.Rating = static_cast<float>(Rating);
                    }
                    
                    // Price level
                    int32 PriceLevel;
                    if (ResultObject->TryGetNumberField(TEXT("price_level"), PriceLevel))
                    {
                        switch (PriceLevel)
                        {
                        case 1: Restaurant.PriceLevel = "$"; break;
                        case 2: Restaurant.PriceLevel = "$$"; break;
                        case 3: Restaurant.PriceLevel = "$$$"; break;
                        case 4: Restaurant.PriceLevel = "$$$$"; break;
                        default: Restaurant.PriceLevel = "N/A"; break;
                        }
                    }
                    
                    // Cuisine types
                    const TArray<TSharedPtr<FJsonValue>>* TypesArray;
                    if (ResultObject->TryGetArrayField(TEXT("types"), TypesArray))
                    {
                        for (const auto& TypeValue : *TypesArray)
                        {
                            FString Type = TypeValue->AsString();
                            if (Type != "restaurant" && Type != "food" && Type != "establishment")
                            {
                                Restaurant.CuisineTypes.Add(Type);
                            }
                        }
                    }
                    
                    Results.Add(Restaurant);
                }
            }
        }
    }
    
    return Results;
}

TArray<FRestaurantData> ARestaurantDataManager::ParseYelpResponse(const FString& ResponseBody)
{
    TArray<FRestaurantData> Results;
    
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
    
    if (FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        const TArray<TSharedPtr<FJsonValue>>* BusinessesArray;
        if (JsonObject->TryGetArrayField(TEXT("businesses"), BusinessesArray))
        {
            for (const auto& BusinessValue : *BusinessesArray)
            {
                TSharedPtr<FJsonObject> BusinessObject = BusinessValue->AsObject();
                if (BusinessObject.IsValid())
                {
                    FRestaurantData Restaurant;
                    
                    // Basic info
                    BusinessObject->TryGetStringField(TEXT("name"), Restaurant.Name);
                    BusinessObject->TryGetStringField(TEXT("id"), Restaurant.YelpBusinessId);
                    BusinessObject->TryGetStringField(TEXT("phone"), Restaurant.PhoneNumber);
                    BusinessObject->TryGetStringField(TEXT("url"), Restaurant.Website);
                    
                    // Location
                    const TSharedPtr<FJsonObject>* CoordinatesObject;
                    if (BusinessObject->TryGetObjectField(TEXT("coordinates"), CoordinatesObject))
                    {
                        double Lat, Lng;
                        if ((*CoordinatesObject)->TryGetNumberField(TEXT("latitude"), Lat) &&
                            (*CoordinatesObject)->TryGetNumberField(TEXT("longitude"), Lng))
                        {
                            Restaurant.Location = FVector2D(Lat, Lng);
                        }
                    }
                    
                    // Address
                    const TSharedPtr<FJsonObject>* LocationObject;
                    if (BusinessObject->TryGetObjectField(TEXT("location"), LocationObject))
                    {
                        FString Address;
                        (*LocationObject)->TryGetStringField(TEXT("address1"), Address);
                        Restaurant.Address = Address;
                    }
                    
                    // Rating and reviews
                    double Rating;
                    if (BusinessObject->TryGetNumberField(TEXT("rating"), Rating))
                    {
                        Restaurant.Rating = static_cast<float>(Rating);
                    }
                    
                    int32 ReviewCount;
                    if (BusinessObject->TryGetNumberField(TEXT("review_count"), ReviewCount))
                    {
                        Restaurant.ReviewCount = ReviewCount;
                    }
                    
                    // Price level
                    FString Price;
                    if (BusinessObject->TryGetStringField(TEXT("price"), Price))
                    {
                        Restaurant.PriceLevel = Price;
                    }
                    
                    // Categories (cuisine types)
                    const TArray<TSharedPtr<FJsonValue>>* CategoriesArray;
                    if (BusinessObject->TryGetArrayField(TEXT("categories"), CategoriesArray))
                    {
                        for (const auto& CategoryValue : *CategoriesArray)
                        {
                            TSharedPtr<FJsonObject> CategoryObject = CategoryValue->AsObject();
                            if (CategoryObject.IsValid())
                            {
                                FString Title;
                                if (CategoryObject->TryGetStringField(TEXT("title"), Title))
                                {
                                    Restaurant.CuisineTypes.Add(Title);
                                }
                            }
                        }
                    }
                    
                    Results.Add(Restaurant);
                }
            }
        }
    }
    
    return Results;
}

void ARestaurantDataManager::MergeRestaurantData(FRestaurantData& Target, const FRestaurantData& Source)
{
    // Merge data from multiple sources, preferring more complete information
    
    if (Target.Rating == 0.0f && Source.Rating > 0.0f)
    {
        Target.Rating = Source.Rating;
    }
    
    if (Target.ReviewCount == 0 && Source.ReviewCount > 0)
    {
        Target.ReviewCount = Source.ReviewCount;
    }
    
    if (Target.PriceLevel.IsEmpty() && !Source.PriceLevel.IsEmpty())
    {
        Target.PriceLevel = Source.PriceLevel;
    }
    
    if (Target.PhoneNumber.IsEmpty() && !Source.PhoneNumber.IsEmpty())
    {
        Target.PhoneNumber = Source.PhoneNumber;
    }
    
    if (Target.Website.IsEmpty() && !Source.Website.IsEmpty())
    {
        Target.Website = Source.Website;
    }
    
    // Merge cuisine types
    for (const FString& CuisineType : Source.CuisineTypes)
    {
        Target.CuisineTypes.AddUnique(CuisineType);
    }
    
    // Store source IDs
    if (!Source.YelpBusinessId.IsEmpty())
    {
        Target.YelpBusinessId = Source.YelpBusinessId;
    }
    
    if (!Source.GooglePlaceId.IsEmpty())
    {
        Target.GooglePlaceId = Source.GooglePlaceId;
    }
}

void ARestaurantDataManager::SortByRelevance(TArray<FRestaurantData>& Restaurants)
{
    Restaurants.Sort([](const FRestaurantData& A, const FRestaurantData& B)
    {
        // Sort by rating first, then by review count
        if (FMath::Abs(A.Rating - B.Rating) > 0.1f)
        {
            return A.Rating > B.Rating;
        }
        
        return A.ReviewCount > B.ReviewCount;
    });
}

FString ARestaurantDataManager::BuildRestaurantContext(const TArray<FRestaurantData>& Restaurants)
{
    FString Context = TEXT("Available restaurants in the area:\n\n");
    
    for (int32 i = 0; i < FMath::Min(Restaurants.Num(), 10); i++)
    {
        const FRestaurantData& Restaurant = Restaurants[i];
        
        Context += FString::Printf(TEXT("%d. %s\n"), i + 1, *Restaurant.Name);
        
        if (!Restaurant.CuisineTypes.IsEmpty())
        {
            Context += FString::Printf(TEXT("   Cuisine: %s\n"), *FString::Join(Restaurant.CuisineTypes, TEXT(", ")));
        }
        
        if (!Restaurant.PriceLevel.IsEmpty())
        {
            Context += FString::Printf(TEXT("   Price: %s\n"), *Restaurant.PriceLevel);
        }
        
        if (Restaurant.Rating > 0.0f)
        {
            Context += FString::Printf(TEXT("   Rating: %.1f/5.0"), Restaurant.Rating);
            if (Restaurant.ReviewCount > 0)
            {
                Context += FString::Printf(TEXT(" (%d reviews)"), Restaurant.ReviewCount);
            }
            Context += TEXT("\n");
        }
        
        if (!Restaurant.Address.IsEmpty())
        {
            Context += FString::Printf(TEXT("   Address: %s\n"), *Restaurant.Address);
        }
        
        Context += TEXT("\n");
    }
    
    return Context;
}

bool ARestaurantDataManager::IsCacheValid(const FString& CacheKey, float MaxAgeMinutes)
{
    if (!RestaurantCache.Contains(CacheKey) || !CacheTimestamps.Contains(CacheKey))
    {
        return false;
    }
    
    FDateTime CacheTime = CacheTimestamps[CacheKey];
    FDateTime Now = FDateTime::Now();
    FTimespan Age = Now - CacheTime;
    
    return Age.GetTotalMinutes() < MaxAgeMinutes;
}

FString ARestaurantDataManager::GenerateCacheKey(FVector2D Location, const FSearchFilters& Filters)
{
    return FString::Printf(TEXT("%.4f_%.4f_%s_%f_%f"), 
        Location.X, Location.Y, 
        *FString::Join(Filters.CuisineTypes, TEXT(",")),
        Filters.MinRating, 
        Filters.MaxDistance);
}

void ARestaurantDataManager::ClearCache()
{
    RestaurantCache.Empty();
    CacheTimestamps.Empty();
    UE_LOG(LogTemp, Log, TEXT("Restaurant cache cleared"));
}

void ARestaurantDataManager::HandleAPIError(const FString& APIName, const FString& ErrorMessage)
{
    UE_LOG(LogTemp, Warning, TEXT("%s API Error: %s"), *APIName, *ErrorMessage);
    OnAPIError.Broadcast(APIName, ErrorMessage);
}