# MetaHuman Setup and Integration

## Overview
Creating and integrating a photorealistic MetaHuman character as the restaurant concierge interface, with natural animations and speech synchronization.

## MetaHuman Creation Process

### 1. Character Design in MetaHuman Creator
**Concierge Persona Characteristics:**
- Professional yet approachable appearance
- Neutral, welcoming facial expression
- Business casual attire suitable for hospitality
- Diverse representation options available

**Recommended Settings:**
```
Body Type: Medium build, professional posture
Face: Friendly, trustworthy features
Hair: Professional, neat styling
Clothing: Business casual (blazer, dress shirt, etc.)
Age Range: 25-40 (appears experienced but approachable)
```

### 2. Export Configuration
```
Quality Level: High (for close-up interactions)
LOD Settings: Multiple levels for performance optimization
Facial Rig: Full facial animation support
Body Rig: Upper body focus (torso, arms, head)
Texture Resolution: 4K for face, 2K for body
Animation Support: Facial expressions, lip sync, gestures
```

## Unreal Engine Integration

### 1. Project Setup
```cpp
// RestaurantConciergePawn.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Animation/AnimInstance.h"
#include "RestaurantConciergePawn.generated.h"

UCLASS()
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

    // Animation and speech functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void PlayGesture(const FString& GestureName);

    UFUNCTION(BlueprintCallable, Category = "Speech")
    void StartSpeaking(USoundWave* AudioClip);

    UFUNCTION(BlueprintCallable, Category = "Speech")
    void StopSpeaking();

    UFUNCTION(BlueprintCallable, Category = "Animation")
    void SetEmotionalState(const FString& Emotion, float Intensity = 1.0f);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    UPROPERTY()
    class UConciergAnimInstance* AnimInstance;

    UPROPERTY()
    bool bIsSpeaking;

    UPROPERTY()
    FString CurrentEmotion;
};
```

### 2. Animation Blueprint Setup
```cpp
// ConciergAnimInstance.h
#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ConciergAnimInstance.generated.h"

UCLASS()
class RESTAURANTCONCIERGE_API UConciergAnimInstance : public UAnimInstance
{
    GENERATED_BODY()

public:
    virtual void NativeInitializeAnimation() override;
    virtual void NativeUpdateAnimation(float DeltaTimeX) override;

    // Animation states
    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool bIsSpeaking;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    bool bIsListening;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    float EmotionIntensity;

    UPROPERTY(BlueprintReadOnly, Category = "Animation")
    FString CurrentGesture;

    // Facial animation
    UPROPERTY(BlueprintReadOnly, Category = "Facial")
    float EyeBlinkRate;

    UPROPERTY(BlueprintReadOnly, Category = "Facial")
    FVector2D EyeLookDirection;

    UPROPERTY(BlueprintReadOnly, Category = "Facial")
    float SmileIntensity;

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Animation")
    void TriggerGesture(const FString& GestureName);

    UFUNCTION(BlueprintCallable, Category = "Facial")
    void SetFacialExpression(const FString& Expression, float Intensity);

private:
    UPROPERTY()
    class ARestaurantConciergePawn* OwnerPawn;
};
```

## Facial Animation and Lip Sync

### 1. Audio2Face Integration (Alternative to built-in)
```cpp
// LipSyncManager.h
class RESTAURANTCONCIERGE_API ULipSyncManager : public UObject
{
public:
    UFUNCTION(BlueprintCallable)
    void ProcessAudioForLipSync(USoundWave* AudioClip);

    UFUNCTION(BlueprintCallable)
    void ApplyLipSyncData(const TArray<float>& VisemeWeights, float TimeStamp);

private:
    // Viseme mapping for MetaHuman
    TMap<FString, int32> VisemeMapping;
    
    void InitializeVisemeMapping();
    TArray<float> ExtractVisemeWeights(USoundWave* Audio, float TimeStamp);
};

// Initialize viseme mapping for MetaHuman facial rig
void ULipSyncManager::InitializeVisemeMapping()
{
    VisemeMapping.Add("sil", 0);    // Silence
    VisemeMapping.Add("PP", 1);     // P, B, M
    VisemeMapping.Add("FF", 2);     // F, V
    VisemeMapping.Add("TH", 3);     // TH
    VisemeMapping.Add("DD", 4);     // T, D, N, L
    VisemeMapping.Add("kk", 5);     // K, G
    VisemeMapping.Add("CH", 6);     // CH, J, SH
    VisemeMapping.Add("SS", 7);     // S, Z
    VisemeMapping.Add("nn", 8);     // N, NG
    VisemeMapping.Add("RR", 9);     // R
    VisemeMapping.Add("aa", 10);    // AA (father)
    VisemeMapping.Add("E", 11);     // E (bed)
    VisemeMapping.Add("I", 12);     // I (bit)
    VisemeMapping.Add("O", 13);     // O (boat)
    VisemeMapping.Add("U", 14);     // U (book)
}
```

### 2. Real-time Facial Animation
```cpp
void UConciergAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);

    if (OwnerPawn)
    {
        // Update speaking state
        bIsSpeaking = OwnerPawn->GetVoiceAudioComponent()->IsPlaying();
        
        // Natural eye movement
        UpdateEyeMovement(DeltaTimeX);
        
        // Breathing animation
        UpdateBreathingAnimation(DeltaTimeX);
        
        // Idle gestures when not speaking
        if (!bIsSpeaking)
        {
            UpdateIdleGestures(DeltaTimeX);
        }
    }
}

void UConciergAnimInstance::UpdateEyeMovement(float DeltaTime)
{
    // Simulate natural eye movement
    static float EyeTimer = 0.0f;
    EyeTimer += DeltaTime;
    
    // Random eye movement every 2-4 seconds
    if (EyeTimer > FMath::RandRange(2.0f, 4.0f))
    {
        EyeLookDirection.X = FMath::RandRange(-0.3f, 0.3f);
        EyeLookDirection.Y = FMath::RandRange(-0.2f, 0.2f);
        EyeTimer = 0.0f;
    }
    
    // Blinking
    static float BlinkTimer = 0.0f;
    BlinkTimer += DeltaTime;
    
    if (BlinkTimer > FMath::RandRange(3.0f, 8.0f))
    {
        // Trigger blink animation
        EyeBlinkRate = 1.0f;
        BlinkTimer = 0.0f;
    }
    else
    {
        EyeBlinkRate = FMath::Max(0.0f, EyeBlinkRate - DeltaTime * 5.0f);
    }
}
```

## Gesture System

### 1. Contextual Gestures
```cpp
// GestureManager.h
USTRUCT(BlueprintType)
struct FGestureData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString GestureName;

    UPROPERTY(BlueprintReadWrite)
    UAnimMontage* Animation;

    UPROPERTY(BlueprintReadWrite)
    TArray<FString> TriggerKeywords;

    UPROPERTY(BlueprintReadWrite)
    float Priority;
};

class RESTAURANTCONCIERGE_API UGestureManager : public UObject
{
public:
    UFUNCTION(BlueprintCallable)
    void InitializeGestures();

    UFUNCTION(BlueprintCallable)
    FString SelectGestureForSpeech(const FString& SpeechText);

    UFUNCTION(BlueprintCallable)
    void PlayGesture(const FString& GestureName);

private:
    UPROPERTY()
    TArray<FGestureData> AvailableGestures;

    UPROPERTY()
    class ARestaurantConciergePawn* OwnerPawn;
};

void UGestureManager::InitializeGestures()
{
    // Welcome gestures
    FGestureData WelcomeGesture;
    WelcomeGesture.GestureName = "Welcome";
    WelcomeGesture.TriggerKeywords = {"hello", "welcome", "greetings", "good morning", "good evening"};
    WelcomeGesture.Priority = 1.0f;
    AvailableGestures.Add(WelcomeGesture);

    // Pointing gestures for directions
    FGestureData PointingGesture;
    PointingGesture.GestureName = "Pointing";
    PointingGesture.TriggerKeywords = {"over there", "that way", "direction", "located", "find"};
    PointingGesture.Priority = 0.8f;
    AvailableGestures.Add(PointingGesture);

    // Explanatory gestures
    FGestureData ExplainingGesture;
    ExplainingGesture.GestureName = "Explaining";
    ExplainingGesture.TriggerKeywords = {"because", "however", "therefore", "explain", "details"};
    ExplainingGesture.Priority = 0.6f;
    AvailableGestures.Add(ExplainingGesture);

    // Counting gestures
    FGestureData CountingGesture;
    CountingGesture.GestureName = "Counting";
    CountingGesture.TriggerKeywords = {"first", "second", "third", "options", "choices"};
    CountingGesture.Priority = 0.7f;
    AvailableGestures.Add(CountingGesture);
}
```

### 2. Emotional States
```cpp
void ARestaurantConciergePawn::SetEmotionalState(const FString& Emotion, float Intensity)
{
    CurrentEmotion = Emotion;
    
    if (AnimInstance)
    {
        AnimInstance->SetFacialExpression(Emotion, Intensity);
    }
    
    // Adjust voice parameters if needed
    if (VoiceAudioComponent)
    {
        // Modify pitch/tone based on emotion
        float PitchMultiplier = 1.0f;
        
        if (Emotion == "Excited")
        {
            PitchMultiplier = 1.1f;
        }
        else if (Emotion == "Sympathetic")
        {
            PitchMultiplier = 0.95f;
        }
        
        VoiceAudioComponent->SetPitchMultiplier(PitchMultiplier);
    }
}
```

## Performance Optimization

### 1. LOD System
```cpp
// MetaHumanLODManager.h
class RESTAURANTCONCIERGE_API UMetaHumanLODManager : public UObject
{
public:
    UFUNCTION(BlueprintCallable)
    void UpdateLODBasedOnDistance(float DistanceToCamera);

    UFUNCTION(BlueprintCallable)
    void SetPerformanceMode(bool bHighPerformance);

private:
    void SetMeshLOD(int32 LODLevel);
    void SetAnimationQuality(int32 QualityLevel);
    void SetTextureQuality(int32 QualityLevel);
};

void UMetaHumanLODManager::UpdateLODBasedOnDistance(float Distance)
{
    int32 LODLevel = 0;
    
    if (Distance > 500.0f)
    {
        LODLevel = 2; // Low detail for distant view
    }
    else if (Distance > 200.0f)
    {
        LODLevel = 1; // Medium detail
    }
    else
    {
        LODLevel = 0; // High detail for close-up
    }
    
    SetMeshLOD(LODLevel);
}
```

### 2. Animation Culling
```cpp
void UConciergAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    // Only update complex animations when character is visible
    if (IsCharacterVisible())
    {
        UpdateDetailedFacialAnimation(DeltaTimeX);
        UpdateFingerAnimations(DeltaTimeX);
    }
    else
    {
        // Simplified updates when not visible
        UpdateBasicAnimation(DeltaTimeX);
    }
}
```

## Blueprint Integration

### 1. Main Character Blueprint (BP_RestaurantConcierge)
```
Components:
├── MetaHuman Skeletal Mesh
├── Audio Component (Voice)
├── Camera Target (Scene Component)
├── Gesture Manager (Custom Component)
└── Lip Sync Manager (Custom Component)

Event Graph:
├── Begin Play → Initialize Systems
├── On Speech Start → Update Animation State
├── On Speech End → Return to Idle
└── Tick → Update Eye Movement & Breathing
```

### 2. Animation Blueprint (ABP_Concierge)
```
State Machine:
├── Idle State
│   ├── Breathing Animation
│   ├── Eye Movement
│   └── Occasional Gestures
├── Speaking State
│   ├── Lip Sync Animation
│   ├── Contextual Gestures
│   └── Facial Expressions
└── Listening State
    ├── Attentive Posture
    ├── Eye Contact
    └── Subtle Nods
```

This comprehensive MetaHuman setup provides a natural, engaging interface for your restaurant concierge that can seamlessly integrate with the Bedrock Nova Sonic speech system and restaurant data APIs.