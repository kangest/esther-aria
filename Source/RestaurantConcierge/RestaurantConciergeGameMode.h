#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RestaurantConciergeGameMode.generated.h"

UCLASS(BlueprintType, Blueprintable)
class RESTAURANTCONCIERGE_API ARestaurantConciergeGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ARestaurantConciergeGameMode();

protected:
    virtual void BeginPlay() override;

    // Core system references
    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    class ARestaurantDataManager* RestaurantDataManager;

    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    class ABedrockAudioManager* BedrockAudioManager;

    UPROPERTY(BlueprintReadOnly, Category = "Systems")
    class ARestaurantConciergePawn* ConciergePawn;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    FString DefaultLocation = "Seattle, WA";

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bAutoStartSystems = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Configuration")
    bool bUseMockData = true;

public:
    // System access functions
    UFUNCTION(BlueprintCallable, Category = "Systems")
    ARestaurantDataManager* GetRestaurantDataManager() const { return RestaurantDataManager; }

    UFUNCTION(BlueprintCallable, Category = "Systems")
    ABedrockAudioManager* GetBedrockAudioManager() const { return BedrockAudioManager; }

    UFUNCTION(BlueprintCallable, Category = "Systems")
    ARestaurantConciergePawn* GetConciergePawn() const { return ConciergePawn; }

    // System initialization
    UFUNCTION(BlueprintCallable, Category = "Initialization")
    void InitializeSystems();

    UFUNCTION(BlueprintCallable, Category = "Initialization")
    void ConnectSystems();

private:
    void SpawnCoreActors();
    void SetupSystemBindings();
    void LoadConfiguration();

    // Event handlers
    UFUNCTION()
    void OnRestaurantsFound(const TArray<FRestaurantData>& Restaurants);

    UFUNCTION()
    void OnRestaurantAPIError(const FString& APIName, const FString& ErrorMessage);

    UFUNCTION()
    void OnSpeechProcessed(const FString& ResponseText);

    UFUNCTION()
    void OnAudioResponseReady(USoundWave* AudioResponse);

    UFUNCTION()
    void OnBedrockError(const FString& ErrorType, const FString& ErrorMessage);
};