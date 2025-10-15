#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Animation/AnimInstance.h"
#include "Sound/SoundWave.h"
#include "RestaurantConciergePawn.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGestureComplete, const FString&, GestureName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnEmotionChanged, const FString&, EmotionName);

UCLASS(BlueprintType, Blueprintable)
class RESTAURANTCONCIERGE_API ARestaurantConciergePawn : public APawn
{
    GENERATED_BODY()

public:
    ARestaurantConciergePawn();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* MetaHumanMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* VoiceAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* CameraTarget;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnGestureComplete OnGestureComplete;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnEmotionChanged OnEmotionChanged;

    // Animation and speech functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayGesture(const FString& GestureName);

    UFUNCTION(BlueprintCallable, Category = "Speech")
    void StartSpeaking(USoundWave* AudioClip);

    UFUNCTION(BlueprintCallable, Category = "Speech")
    void StopSpeaking();

    UFUNCTION(BlueprintCallable, Category = "Speech")
    bool IsSpeaking() const;

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetEmotionalState(const FString& Emotion, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetListeningState(bool bIsListening);

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerIdleGesture();

    UFUNCTION(BlueprintCallable, Category = "Facial")
    void SetFacialExpression(const FString& Expression, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Eyes")
    void SetEyeLookTarget(FVector WorldLocation);

    UFUNCTION(BlueprintCallable, Category = "Eyes")
    void ResetEyeLook();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY()
    class UConciergAnimInstance* AnimInstance;

    UPROPERTY()
    bool bIsSpeaking = false;

    UPROPERTY()
    bool bIsListening = false;

    UPROPERTY()
    FString CurrentEmotion = "Neutral";

    UPROPERTY()
    float CurrentEmotionIntensity = 1.0f;

    UPROPERTY()
    FVector EyeLookTarget = FVector::ZeroVector;

    UPROPERTY()
    bool bHasEyeLookTarget = false;

    // Gesture system
    UPROPERTY(EditAnywhere, Category = "Gestures", meta = (AllowPrivateAccess = "true"))
    TMap<FString, class UAnimMontage*> GestureAnimations;

    UPROPERTY(EditAnywhere, Category = "Facial Expressions", meta = (AllowPrivateAccess = "true"))
    TMap<FString, class UAnimMontage*> FacialExpressions;

    // Idle behavior
    UPROPERTY(EditAnywhere, Category = "Idle Behavior", meta = (AllowPrivateAccess = "true"))
    TArray<FString> IdleGestures;

    UPROPERTY(EditAnywhere, Category = "Idle Behavior", meta = (AllowPrivateAccess = "true"))
    float IdleGestureFrequency = 15.0f; // seconds between idle gestures

    UPROPERTY()
    float LastIdleGestureTime = 0.0f;

    // Eye movement
    UPROPERTY(EditAnywhere, Category = "Eye Movement", meta = (AllowPrivateAccess = "true"))
    float EyeMovementSpeed = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Eye Movement", meta = (AllowPrivateAccess = "true"))
    float BlinkFrequency = 5.0f; // seconds between blinks

    UPROPERTY()
    float LastBlinkTime = 0.0f;

    // Audio callbacks
    UFUNCTION()
    void OnAudioFinished();

    // Animation callbacks
    UFUNCTION()
    void OnGestureAnimationComplete(UAnimMontage* Montage, bool bInterrupted);

    // Utility functions
    void UpdateIdleBehavior(float DeltaTime);
    void UpdateEyeMovement(float DeltaTime);
    void UpdateBreathing(float DeltaTime);
    FString SelectContextualGesture(const FString& SpeechText);
    void InitializeGestures();
    void InitializeFacialExpressions();
};