#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ConciergAnimInstance.generated.h"

UCLASS(BlueprintType, Blueprintable)
class RESTAURANTCONCIERGE_API UConciergAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Animation states
    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    bool bIsSpeaking = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    bool bIsListening = false;

    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    float EmotionIntensity = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Animation States")
    FString CurrentEmotion = "Neutral";

    // Facial animation
    UPROPERTY(BlueprintReadOnly, Category = "Facial Animation")
    float EyeBlinkWeight = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Facial Animation")
    FVector2D EyeLookDirection = FVector2D::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category = "Facial Animation")
    float SmileIntensity = 0.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Facial Animation")
    float BrowRaiseIntensity = 0.0f;

    // Body animation
    UPROPERTY(BlueprintReadOnly, Category = "Body Animation")
    float BreathingIntensity = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Body Animation")
    float PostureWeight = 1.0f;

    UPROPERTY(BlueprintReadOnly, Category = "Body Animation")
    bool bIsGesturing = false;

    // Lip sync
    UPROPERTY(BlueprintReadOnly, Category = "Lip Sync")
    TArray<float> VisemeWeights;

    // Functions callable from C++
    void SetSpeakingState(bool bSpeaking);
    void SetListeningState(bool bListening);
    void SetEmotionalState(const FString& Emotion, float Intensity);
    void SetEyeLookTarget(FVector WorldLocation);
    void ResetEyeLook();
    void TriggerBlink();
    void UpdateBreathing(float DeltaTime);
    void UpdateNaturalEyeMovement(float DeltaTime);
    void SetOwnerPawn(class ARestaurantConciergePawn* Pawn);

    // Blueprint callable functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerGesture(const FString& GestureName);

    UFUNCTION(BlueprintCallable, Category = "Facial Animation")
    void SetFacialExpression(const FString& Expression, float Intensity);

    UFUNCTION(BlueprintCallable, Category = "Lip Sync")
    void UpdateLipSync(const TArray<float>& NewVisemeWeights);

protected:
    UPROPERTY()
    class ARestaurantConciergePawn* OwnerPawn;

private:
    // Eye movement variables
    FVector CurrentEyeLookTarget = FVector::ZeroVector;
    bool bHasEyeLookTarget = false;
    FVector2D NaturalEyeDirection = FVector2D::ZeroVector;
    float EyeMovementTimer = 0.0f;
    float NextEyeMovementTime = 0.0f;

    // Blinking variables
    float BlinkTimer = 0.0f;
    float BlinkDuration = 0.15f;
    bool bIsBlinking = false;

    // Breathing variables
    float BreathingTimer = 0.0f;
    float BreathingRate = 0.2f; // Breaths per second

    // Emotion blending
    FString PreviousEmotion = "Neutral";
    float EmotionBlendTime = 0.0f;
    float EmotionBlendDuration = 1.0f;

    // Utility functions
    void UpdateEyeBlinking(float DeltaTime);
    void UpdateEyeLookDirection(float DeltaTime);
    void UpdateEmotionBlending(float DeltaTime);
    void UpdateFacialExpressions(float DeltaTime);
    void ApplyEmotionalModifiers();
    FVector2D CalculateEyeLookDirection(FVector WorldTarget);
    float GetEmotionWeight(const FString& EmotionName);
};