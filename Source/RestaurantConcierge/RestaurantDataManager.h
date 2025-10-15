#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "Http.h"
#include "RestaurantData.h"
#include "RestaurantDataManager.generated.h"

UCLASS(BlueprintType, Blueprintable)
class RESTAURANTCONCIERGE_API ARestaurantDataManager : public AActor
{
    GENERATED_BODY()

public:
    ARestaurantDataManager();

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnRestaurantsFound OnRestaurantsFound;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnAPIError OnAPIError;

    UFUNCTION(BlueprintCallable, Category = "Restaurant Search")
    void SearchRestaurants(FVector2D Location, const FSearchFilters& Filters);

    UFUNCTION(BlueprintCallable, Category = "Restaurant Search")
    void GetRestaurantDetails(const FString& RestaurantId, const FString& APISource = "GooglePlaces");

    UFUNCTION(BlueprintCallable, Category = "Restaurant Search")
    FString BuildRestaurantContext(const TArray<FRestaurantData>& Restaurants);

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void SetAPIKeys(const FString& GooglePlacesKey, const FString& YelpKey);

    UFUNCTION(BlueprintCallable, Category = "Cache")
    void ClearCache();

protected:
    virtual void BeginPlay() override;

private:
    // API Keys
    UPROPERTY(EditAnywhere, Category = "API Configuration", meta = (AllowPrivateAccess = "true"))
    FString GooglePlacesAPIKey;

    UPROPERTY(EditAnywhere, Category = "API Configuration", meta = (AllowPrivateAccess = "true"))
    FString YelpAPIKey;

    // API Base URLs
    FString GooglePlacesBaseURL = "https://maps.googleapis.com/maps/api/place/";
    FString YelpBaseURL = "https://api.yelp.com/v3/";

    // Current search data
    UPROPERTY()
    TArray<FRestaurantData> CurrentRestaurants;

    UPROPERTY()
    FVector2D CurrentSearchLocation;

    UPROPERTY()
    FSearchFilters CurrentFilters;

    // Cache system
    UPROPERTY()
    TMap<FString, TArray<FRestaurantData>> RestaurantCache;

    UPROPERTY()
    TMap<FString, FDateTime> CacheTimestamps;

    // HTTP request handling
    void OnGooglePlacesResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void OnYelpResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    void OnRestaurantDetailsResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);
    
    // Search methods
    void SearchGooglePlaces(FVector2D Location, const FSearchFilters& Filters);
    void SearchYelp(FVector2D Location, const FSearchFilters& Filters);
    void CheckRequestsComplete();

    // API request builders
    FString BuildGooglePlacesSearchURL(FVector2D Location, const FSearchFilters& Filters);
    FString BuildYelpSearchURL(FVector2D Location, const FSearchFilters& Filters);
    FString BuildGooglePlaceDetailsURL(const FString& PlaceId);

    // Data processing
    TArray<FRestaurantData> ParseGooglePlacesResponse(const FString& ResponseBody);
    TArray<FRestaurantData> ParseYelpResponse(const FString& ResponseBody);
    FRestaurantData ParseGooglePlaceDetails(const FString& ResponseBody);

    // Utility functions
    void CombineSearchResults(const TArray<FRestaurantData>& GoogleResults, const TArray<FRestaurantData>& YelpResults);
    void MergeRestaurantData(FRestaurantData& Target, const FRestaurantData& Source);
    void SortByRelevance(TArray<FRestaurantData>& Restaurants);
    bool IsCacheValid(const FString& CacheKey, float MaxAgeMinutes = 30.0f);
    FString GenerateCacheKey(FVector2D Location, const FSearchFilters& Filters);
    FString GetTodayHours(const FOperatingHours& Hours);

    // Error handling
    void HandleAPIError(const FString& APIName, const FString& ErrorMessage);

    // Request tracking
    int32 PendingRequests = 0;
    bool bGooglePlacesComplete = false;
    bool bYelpComplete = false;
};