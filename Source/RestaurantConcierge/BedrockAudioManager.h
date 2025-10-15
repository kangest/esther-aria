#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "RestaurantData.h"
#include "BedrockAudioManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSpeechProcessed, const FString&, ResponseText);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAudioResponseReady, USoundWave*, AudioResponse);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBedrockError, const FString&, ErrorType, const FString&, ErrorMessage);

UCLASS(BlueprintType, Blueprintable)
class RESTAURANTCONCIERGE_API ABedrockAudioManager : public AActor
{
    GENERATED_BODY()

public:
    ABedrockAudioManager();

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnSpeechProcessed OnSpeechProcessed;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnAudioResponseReady OnAudioResponseReady;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnBedrockError OnBedrockError;

    UFUNCTION(BlueprintCallable, Category = "Speech Processing")
    void ProcessSpeechInput(const TArray<uint8>& AudioData);

    UFUNCTION(BlueprintCallable, Category = "Speech Processing")
    void ProcessTextInput(const FString& InputText);

    UFUNCTION(BlueprintCallable, Category = "Context")
    void SetRestaurantContext(const FString& Location, const TArray<FRestaurantData>& Restaurants);

    UFUNCTION(BlueprintCallable, Category = "Context")
    void UpdateUserPreferences(const TArray<FString>& Preferences);

    UFUNCTION(BlueprintCallable, Category = "Configuration")
    void SetBedrockConfiguration(const FString& Region, const FString& ModelId);

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StartListening();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    void StopListening();

    UFUNCTION(BlueprintCallable, Category = "Audio")
    bool IsListening() const { return bIsListening; }

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    // Configuration
    UPROPERTY(EditAnywhere, Category = "Bedrock Configuration", meta = (AllowPrivateAccess = "true"))
    FString BedrockRegion = "us-east-1";

    UPROPERTY(EditAnywhere, Category = "Bedrock Configuration", meta = (AllowPrivateAccess = "true"))
    FString BedrockModelId = "amazon.nova-sonic-v1:0";

    UPROPERTY(EditAnywhere, Category = "Audio Configuration", meta = (AllowPrivateAccess = "true"))
    int32 SampleRate = 16000;

    UPROPERTY(EditAnywhere, Category = "Audio Configuration", meta = (AllowPrivateAccess = "true"))
    int32 Channels = 1;

    UPROPERTY(EditAnywhere, Category = "Audio Configuration", meta = (AllowPrivateAccess = "true"))
    float MaxRecordingDuration = 30.0f;

    // Audio components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
    UAudioComponent* AudioOutputComponent;

    // State management
    UPROPERTY()
    bool bIsListening = false;

    UPROPERTY()
    bool bIsProcessing = false;

    UPROPERTY()
    FString CurrentLocation;

    UPROPERTY()
    TArray<FRestaurantData> CurrentRestaurants;

    UPROPERTY()
    TArray<FString> UserPreferences;

    UPROPERTY()
    FString RestaurantContext;

    // Audio processing
    UPROPERTY()
    TArray<uint8> AudioBuffer;

    UPROPERTY()
    float RecordingStartTime = 0.0f;

    // Voice Activity Detection
    UPROPERTY()
    float SilenceThreshold = 0.01f;

    UPROPERTY()
    float SilenceDuration = 0.0f;

    UPROPERTY()
    float MaxSilenceDuration = 2.0f;

    // HTTP request handling
    void SendBedrockRequest(const FString& RequestBody);
    void OnBedrockResponse(class FHttpRequestPtr Request, class FHttpResponsePtr Response, bool bWasSuccessful);

    // Audio processing methods
    TArray<uint8> ConvertAudioToFormat(const TArray<uint8>& InputAudio);
    FString EncodeAudioToBase64(const TArray<uint8>& AudioData);
    USoundWave* DecodeAudioFromBase64(const FString& Base64Audio);
    bool DetectVoiceActivity(const TArray<uint8>& AudioData);
    float CalculateAudioLevel(const TArray<uint8>& AudioData);

    // Request building
    FString BuildBedrockRequestBody(const FString& InputText = "", const FString& AudioBase64 = "");
    FString BuildSystemPrompt();
    FString BuildRestaurantPrompt(const FString& UserInput);

    // Response processing
    void ProcessBedrockResponse(const FString& ResponseBody);
    void HandleBedrockError(const FString& ErrorType, const FString& ErrorMessage);

    // Utility methods
    void InitializeAudioCapture();
    void CleanupAudioCapture();
    void ResetAudioBuffer();

    // Mock Bedrock implementation (for development without AWS)
    UPROPERTY(EditAnywhere, Category = "Development", meta = (AllowPrivateAccess = "true"))
    bool bUseMockBedrock = true;

    void ProcessMockBedrock(const FString& InputText);
    FString GenerateMockResponse(const FString& InputText);
};