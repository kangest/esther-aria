#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RestaurantData.generated.h"

USTRUCT(BlueprintType)
struct FOperatingHours
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Hours")
    TMap<FString, FString> WeeklyHours; // "Monday" -> "9:00 AM - 10:00 PM"

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Hours")
    bool bOpen24Hours = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Hours")
    bool bTemporarilyClosed = false;

    FOperatingHours()
    {
        WeeklyHours.Add("Monday", "Closed");
        WeeklyHours.Add("Tuesday", "Closed");
        WeeklyHours.Add("Wednesday", "Closed");
        WeeklyHours.Add("Thursday", "Closed");
        WeeklyHours.Add("Friday", "Closed");
        WeeklyHours.Add("Saturday", "Closed");
        WeeklyHours.Add("Sunday", "Closed");
    }
};

USTRUCT(BlueprintType)
struct FRestaurantData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Basic Info")
    FString Name;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Basic Info")
    FString Address;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Location")
    FVector2D Location; // Latitude, Longitude

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Cuisine")
    TArray<FString> CuisineTypes;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Pricing")
    FString PriceLevel; // $, $$, $$$, $$$$

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Rating")
    float Rating = 0.0f; // 1.0 - 5.0

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Rating")
    int32 ReviewCount = 0;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Hours")
    FOperatingHours Hours;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Media")
    TArray<FString> PhotoURLs;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Contact")
    FString PhoneNumber;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Contact")
    FString Website;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Services")
    bool bAcceptsReservations = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Services")
    bool bTakeout = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Services")
    bool bDelivery = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "API Data")
    FString GooglePlaceId;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "API Data")
    FString YelpBusinessId;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Internal")
    float DistanceFromUser = 0.0f; // In meters

    FRestaurantData()
    {
        Location = FVector2D::ZeroVector;
        Rating = 0.0f;
        ReviewCount = 0;
        bAcceptsReservations = false;
        bTakeout = false;
        bDelivery = false;
        DistanceFromUser = 0.0f;
    }
};

USTRUCT(BlueprintType)
struct FSearchFilters
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Filters")
    TArray<FString> CuisineTypes;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Filters")
    FString PriceRange; // "$-$$", "$$-$$$", etc.

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Filters")
    float MinRating = 0.0f;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Filters")
    float MaxDistance = 5000.0f; // meters

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Filters")
    bool bOpenNow = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Filters")
    bool bAcceptsReservations = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Filters")
    bool bHasDelivery = false;

    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Filters")
    bool bHasTakeout = false;

    FSearchFilters()
    {
        MinRating = 0.0f;
        MaxDistance = 5000.0f;
        bOpenNow = false;
        bAcceptsReservations = false;
        bHasDelivery = false;
        bHasTakeout = false;
    }
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRestaurantsFound, const TArray<FRestaurantData>&, Restaurants);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAPIError, const FString&, APIName, const FString&, ErrorMessage);