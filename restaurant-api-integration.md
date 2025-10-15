# Restaurant API Integration

## Overview
Integration with multiple restaurant data sources to provide comprehensive information including hours, cuisine, pricing, ratings, and photos.

## Supported APIs

### 1. Google Places API
**Best for**: Comprehensive data, photos, reviews
```cpp
// GooglePlacesAPI.h
class RESTAURANTCONCIERGE_API UGooglePlacesAPI : public UObject
{
public:
    UFUNCTION(BlueprintCallable)
    void SearchNearbyRestaurants(FVector2D Location, float Radius = 5000.0f);
    
    UFUNCTION(BlueprintCallable)
    void GetRestaurantDetails(const FString& PlaceId);
    
    UFUNCTION(BlueprintCallable)
    void GetRestaurantPhotos(const FString& PlaceId);

private:
    FString APIKey;
    FString BaseURL = "https://maps.googleapis.com/maps/api/place/";
};
```

### 2. Yelp Fusion API
**Best for**: Reviews, ratings, pricing information
```cpp
// YelpAPI.h
class RESTAURANTCONCIERGE_API UYelpAPI : public UObject
{
public:
    UFUNCTION(BlueprintCallable)
    void SearchBusinesses(const FString& Location, const FString& Categories = "restaurants");
    
    UFUNCTION(BlueprintCallable)
    void GetBusinessDetails(const FString& BusinessId);

private:
    FString APIKey;
    FString BaseURL = "https://api.yelp.com/v3/";
};
```

### 3. OpenTable API (if available)
**Best for**: Reservation availability, special events

## Data Models

### Restaurant Data Structure
```cpp
USTRUCT(BlueprintType)
struct FRestaurantData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString Name;
    
    UPROPERTY(BlueprintReadWrite)
    FString Address;
    
    UPROPERTY(BlueprintReadWrite)
    FVector2D Location; // Lat, Lng
    
    UPROPERTY(BlueprintReadWrite)
    TArray<FString> CuisineTypes;
    
    UPROPERTY(BlueprintReadWrite)
    FString PriceLevel; // $, $$, $$$, $$$$
    
    UPROPERTY(BlueprintReadWrite)
    float Rating; // 1.0 - 5.0
    
    UPROPERTY(BlueprintReadWrite)
    int32 ReviewCount;
    
    UPROPERTY(BlueprintReadWrite)
    FOperatingHours Hours;
    
    UPROPERTY(BlueprintReadWrite)
    TArray<FString> PhotoURLs;
    
    UPROPERTY(BlueprintReadWrite)
    FString PhoneNumber;
    
    UPROPERTY(BlueprintReadWrite)
    FString Website;
    
    UPROPERTY(BlueprintReadWrite)
    bool bAcceptsReservations;
    
    UPROPERTY(BlueprintReadWrite)
    bool bTakeout;
    
    UPROPERTY(BlueprintReadWrite)
    bool bDelivery;
};

USTRUCT(BlueprintType)
struct FOperatingHours
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadWrite)
    TMap<FString, FString> WeeklyHours; // "Monday" -> "9:00 AM - 10:00 PM"
    
    UPROPERTY(BlueprintReadWrite)
    bool bOpen24Hours;
    
    UPROPERTY(BlueprintReadWrite)
    bool bTemporarilyClosed;
};
```

## Implementation

### 1. Restaurant Data Manager
```cpp
// RestaurantDataManager.cpp
void URestaurantDataManager::SearchRestaurants(FVector2D Location, const FSearchFilters& Filters)
{
    // Parallel API calls for comprehensive data
    TArray<TFuture<TArray<FRestaurantData>>> APIFutures;
    
    // Google Places search
    APIFutures.Add(Async(EAsyncExecution::ThreadPool, [this, Location, Filters]()
    {
        return GooglePlacesAPI->SearchNearby(Location, Filters);
    }));
    
    // Yelp search
    APIFutures.Add(Async(EAsyncExecution::ThreadPool, [this, Location, Filters]()
    {
        return YelpAPI->SearchBusinesses(Location, Filters);
    }));
    
    // Combine results when all complete
    CombineSearchResults(APIFutures);
}

void URestaurantDataManager::CombineSearchResults(const TArray<TFuture<TArray<FRestaurantData>>>& Futures)
{
    TArray<FRestaurantData> CombinedResults;
    TMap<FString, FRestaurantData> DeduplicatedResults;
    
    for (const auto& Future : Futures)
    {
        TArray<FRestaurantData> Results = Future.Get();
        
        for (const FRestaurantData& Restaurant : Results)
        {
            FString Key = Restaurant.Name + Restaurant.Address;
            
            if (DeduplicatedResults.Contains(Key))
            {
                // Merge data from multiple sources
                MergeRestaurantData(DeduplicatedResults[Key], Restaurant);
            }
            else
            {
                DeduplicatedResults.Add(Key, Restaurant);
            }
        }
    }
    
    // Convert to array and sort by relevance
    DeduplicatedResults.GenerateValueArray(CombinedResults);
    SortByRelevance(CombinedResults);
    
    OnRestaurantsFound.Broadcast(CombinedResults);
}
```

### 2. Search Filters
```cpp
USTRUCT(BlueprintType)
struct FSearchFilters
{
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadWrite)
    TArray<FString> CuisineTypes;
    
    UPROPERTY(BlueprintReadWrite)
    FString PriceRange; // "$-$$", "$$-$$$", etc.
    
    UPROPERTY(BlueprintReadWrite)
    float MinRating = 0.0f;
    
    UPROPERTY(BlueprintReadWrite)
    float MaxDistance = 5000.0f; // meters
    
    UPROPERTY(BlueprintReadWrite)
    bool bOpenNow = false;
    
    UPROPERTY(BlueprintReadWrite)
    bool bAcceptsReservations = false;
    
    UPROPERTY(BlueprintReadWrite)
    bool bHasDelivery = false;
    
    UPROPERTY(BlueprintReadWrite)
    bool bHasTakeout = false;
};
```

### 3. Caching System
```cpp
class RESTAURANTCONCIERGE_API URestaurantCache : public UObject
{
public:
    void CacheRestaurantData(const FString& LocationKey, const TArray<FRestaurantData>& Restaurants);
    TArray<FRestaurantData> GetCachedData(const FString& LocationKey);
    bool IsCacheValid(const FString& LocationKey, float MaxAgeMinutes = 30.0f);
    
private:
    UPROPERTY()
    TMap<FString, FCachedRestaurantData> Cache;
};

USTRUCT()
struct FCachedRestaurantData
{
    GENERATED_BODY()
    
    UPROPERTY()
    TArray<FRestaurantData> Restaurants;
    
    UPROPERTY()
    FDateTime CacheTime;
};
```

## API Configuration

### 1. Google Places Setup
```cpp
// In BeginPlay or initialization
void UGooglePlacesAPI::Initialize()
{
    APIKey = GetDefault<URestaurantSettings>()->GooglePlacesAPIKey;
    
    // Verify API key is valid
    if (APIKey.IsEmpty())
    {
        UE_LOG(LogRestaurant, Error, TEXT("Google Places API key not configured"));
    }
}

FString UGooglePlacesAPI::BuildSearchURL(FVector2D Location, const FSearchFilters& Filters)
{
    FString URL = BaseURL + "nearbysearch/json?";
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
    
    URL += TEXT("&key=") + APIKey;
    
    return URL;
}
```

### 2. Error Handling
```cpp
void URestaurantDataManager::HandleAPIError(const FString& APIName, const FString& ErrorMessage)
{
    UE_LOG(LogRestaurant, Warning, TEXT("%s API Error: %s"), *APIName, *ErrorMessage);
    
    // Try fallback data sources
    if (APIName == "GooglePlaces")
    {
        // Fall back to Yelp or cached data
        TryFallbackAPIs();
    }
    
    // Notify user of limited data availability
    OnAPIError.Broadcast(APIName, ErrorMessage);
}
```

## Usage in Bedrock Context

### 1. Restaurant Context Builder
```cpp
FString URestaurantDataManager::BuildRestaurantContext(const TArray<FRestaurantData>& Restaurants)
{
    FString Context = TEXT("Available restaurants in the area:\n\n");
    
    for (int32 i = 0; i < FMath::Min(Restaurants.Num(), 10); i++)
    {
        const FRestaurantData& Restaurant = Restaurants[i];
        
        Context += FString::Printf(TEXT("%d. %s\n"), i + 1, *Restaurant.Name);
        Context += FString::Printf(TEXT("   Cuisine: %s\n"), *FString::Join(Restaurant.CuisineTypes, TEXT(", ")));
        Context += FString::Printf(TEXT("   Price: %s\n"), *Restaurant.PriceLevel);
        Context += FString::Printf(TEXT("   Rating: %.1f/5.0 (%d reviews)\n"), Restaurant.Rating, Restaurant.ReviewCount);
        Context += FString::Printf(TEXT("   Address: %s\n"), *Restaurant.Address);
        
        if (!Restaurant.Hours.WeeklyHours.IsEmpty())
        {
            FString TodayHours = GetTodayHours(Restaurant.Hours);
            Context += FString::Printf(TEXT("   Hours today: %s\n"), *TodayHours);
        }
        
        Context += TEXT("\n");
    }
    
    return Context;
}
```

### 2. Integration with Speech System
```cpp
void ABedrockAudioManager::UpdateRestaurantContext(FVector2D Location)
{
    RestaurantDataManager->SearchRestaurants(Location, CurrentFilters);
    
    RestaurantDataManager->OnRestaurantsFound.AddDynamic(this, &ABedrockAudioManager::OnRestaurantsLoaded);
}

void ABedrockAudioManager::OnRestaurantsLoaded(const TArray<FRestaurantData>& Restaurants)
{
    CurrentRestaurants = Restaurants;
    RestaurantContext = RestaurantDataManager->BuildRestaurantContext(Restaurants);
    
    // Update Bedrock system prompt with new restaurant data
    UpdateBedrockContext();
}
```