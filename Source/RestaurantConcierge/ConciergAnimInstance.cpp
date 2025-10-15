#include "ConciergAnimInstance.h"
#include "RestaurantConciergePawn.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"

void UConciergAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    // Initialize viseme weights array for lip sync
    VisemeWeights.SetNum(15); // Standard viseme count
    for (int32 i = 0; i < VisemeWeights.Num(); i++)
    {
        VisemeWeights[i] = 0.0f;
    }

    // Initialize default values
    CurrentEmotion = "Neutral";
    EmotionIntensity = 1.0f;
    BreathingIntensity = 1.0f;
    PostureWeight = 1.0f;
    
    UE_LOG(LogTemp, Log, TEXT("ConciergAnimInstance initialized"));
}

void UConciergAnimInstance::NativeUpdateAnimation(float DeltaTimeX)
{
    Super::NativeUpdateAnimation(DeltaTimeX);

    if (!OwnerPawn)
    {
        OwnerPawn = Cast<ARestaurantConciergePawn>(GetOwningActor());
    }

    // Update various animation systems
    UpdateEyeBlinking(DeltaTimeX);
    UpdateEyeLookDirection(DeltaTimeX);
    UpdateEmotionBlending(DeltaTimeX);
    UpdateFacialExpressions(DeltaTimeX);
    
    // Apply emotional modifiers to all animations
    ApplyEmotionalModifiers();
}

void UConciergAnimInstance::SetSpeakingState(bool bSpeaking)
{
    bIsSpeaking = bSpeaking;
    
    if (bIsSpeaking)
    {
        // Adjust facial expressions for speaking
        SmileIntensity = FMath::Max(SmileIntensity, 0.2f);
        BrowRaiseIntensity = 0.1f;
    }
    else
    {
        // Reset lip sync weights when not speaking
        for (int32 i = 0; i < VisemeWeights.Num(); i++)
        {
            VisemeWeights[i] = 0.0f;
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Animation speaking state: %s"), bSpeaking ? TEXT("True") : TEXT("False"));
}

void UConciergAnimInstance::SetListeningState(bool bListening)
{
    bIsListening = bListening;
    
    if (bIsListening)
    {
        // Attentive posture and facial expression
        PostureWeight = 1.2f; // Slightly more upright
        BrowRaiseIntensity = 0.3f; // Raised eyebrows for attention
        SmileIntensity = 0.1f; // Subtle smile
    }
    else
    {
        // Return to neutral
        PostureWeight = 1.0f;
        BrowRaiseIntensity = 0.0f;
    }
    
    UE_LOG(LogTemp, Log, TEXT("Animation listening state: %s"), bListening ? TEXT("True") : TEXT("False"));
}

void UConciergAnimInstance::SetEmotionalState(const FString& Emotion, float Intensity)
{
    if (CurrentEmotion != Emotion)
    {
        PreviousEmotion = CurrentEmotion;
        CurrentEmotion = Emotion;
        EmotionBlendTime = 0.0f; // Start blending
    }
    
    EmotionIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    
    UE_LOG(LogTemp, Log, TEXT("Animation emotion set: %s (Intensity: %.2f)"), *Emotion, Intensity);
}

void UConciergAnimInstance::SetEyeLookTarget(FVector WorldLocation)
{
    CurrentEyeLookTarget = WorldLocation;
    bHasEyeLookTarget = true;
}

void UConciergAnimInstance::ResetEyeLook()
{
    bHasEyeLookTarget = false;
    CurrentEyeLookTarget = FVector::ZeroVector;
}

void UConciergAnimInstance::TriggerBlink()
{
    if (!bIsBlinking)
    {
        bIsBlinking = true;
        BlinkTimer = 0.0f;
    }
}

void UConciergAnimInstance::UpdateBreathing(float DeltaTime)
{
    BreathingTimer += DeltaTime;
    
    // Sine wave breathing pattern
    float BreathingCycle = FMath::Sin(BreathingTimer * BreathingRate * 2.0f * PI);
    BreathingIntensity = 0.8f + (BreathingCycle * 0.2f); // Subtle breathing variation
    
    // Adjust breathing based on emotional state
    if (CurrentEmotion == "Excited")
    {
        BreathingRate = 0.25f; // Faster breathing
    }
    else if (CurrentEmotion == "Calm" || CurrentEmotion == "Relaxed")
    {
        BreathingRate = 0.15f; // Slower breathing
    }
    else
    {
        BreathingRate = 0.2f; // Normal breathing
    }
}

void UConciergAnimInstance::UpdateNaturalEyeMovement(float DeltaTime)
{
    EyeMovementTimer += DeltaTime;
    
    // Change eye direction periodically
    if (EyeMovementTimer >= NextEyeMovementTime)
    {
        // Generate new random eye direction
        NaturalEyeDirection.X = FMath::RandRange(-0.3f, 0.3f);
        NaturalEyeDirection.Y = FMath::RandRange(-0.2f, 0.2f);
        
        // Set next movement time (2-6 seconds)
        NextEyeMovementTime = FMath::RandRange(2.0f, 6.0f);
        EyeMovementTimer = 0.0f;
    }
    
    // Smoothly interpolate to natural eye direction
    EyeLookDirection = FMath::Vector2DInterpTo(EyeLookDirection, NaturalEyeDirection, DeltaTime, 2.0f);
}

void UConciergAnimInstance::SetOwnerPawn(ARestaurantConciergePawn* Pawn)
{
    OwnerPawn = Pawn;
}

void UConciergAnimInstance::TriggerGesture(const FString& GestureName)
{
    if (OwnerPawn)
    {
        OwnerPawn->PlayGesture(GestureName);
    }
}

void UConciergAnimInstance::SetFacialExpression(const FString& Expression, float Intensity)
{
    if (OwnerPawn)
    {
        OwnerPawn->SetFacialExpression(Expression, Intensity);
    }
}

void UConciergAnimInstance::UpdateLipSync(const TArray<float>& NewVisemeWeights)
{
    if (NewVisemeWeights.Num() == VisemeWeights.Num())
    {
        VisemeWeights = NewVisemeWeights;
    }
}

void UConciergAnimInstance::UpdateEyeBlinking(float DeltaTime)
{
    if (bIsBlinking)
    {
        BlinkTimer += DeltaTime;
        
        // Blink animation curve (quick close, slower open)
        if (BlinkTimer < BlinkDuration * 0.3f)
        {
            // Closing phase
            float BlinkProgress = BlinkTimer / (BlinkDuration * 0.3f);
            EyeBlinkWeight = FMath::InterpEaseOut(0.0f, 1.0f, BlinkProgress, 2.0f);
        }
        else if (BlinkTimer < BlinkDuration)
        {
            // Opening phase
            float BlinkProgress = (BlinkTimer - BlinkDuration * 0.3f) / (BlinkDuration * 0.7f);
            EyeBlinkWeight = FMath::InterpEaseIn(1.0f, 0.0f, BlinkProgress, 2.0f);
        }
        else
        {
            // Blink complete
            EyeBlinkWeight = 0.0f;
            bIsBlinking = false;
            BlinkTimer = 0.0f;
        }
    }
}

void UConciergAnimInstance::UpdateEyeLookDirection(float DeltaTime)
{
    if (bHasEyeLookTarget && OwnerPawn)
    {
        // Calculate eye look direction based on world target
        FVector2D TargetDirection = CalculateEyeLookDirection(CurrentEyeLookTarget);
        EyeLookDirection = FMath::Vector2DInterpTo(EyeLookDirection, TargetDirection, DeltaTime, 3.0f);
    }
}

void UConciergAnimInstance::UpdateEmotionBlending(float DeltaTime)
{
    if (EmotionBlendTime < EmotionBlendDuration)
    {
        EmotionBlendTime += DeltaTime;
        EmotionBlendTime = FMath::Min(EmotionBlendTime, EmotionBlendDuration);
    }
}

void UConciergAnimInstance::UpdateFacialExpressions(float DeltaTime)
{
    // Update facial expressions based on current emotion
    float EmotionWeight = GetEmotionWeight(CurrentEmotion);
    
    if (CurrentEmotion == "Happy" || CurrentEmotion == "Excited")
    {
        SmileIntensity = FMath::FInterpTo(SmileIntensity, 0.7f * EmotionWeight, DeltaTime, 2.0f);
        BrowRaiseIntensity = FMath::FInterpTo(BrowRaiseIntensity, 0.3f * EmotionWeight, DeltaTime, 2.0f);
    }
    else if (CurrentEmotion == "Surprised")
    {
        SmileIntensity = FMath::FInterpTo(SmileIntensity, 0.2f * EmotionWeight, DeltaTime, 3.0f);
        BrowRaiseIntensity = FMath::FInterpTo(BrowRaiseIntensity, 0.8f * EmotionWeight, DeltaTime, 3.0f);
    }
    else if (CurrentEmotion == "Concerned" || CurrentEmotion == "Sympathetic")
    {
        SmileIntensity = FMath::FInterpTo(SmileIntensity, 0.0f, DeltaTime, 2.0f);
        BrowRaiseIntensity = FMath::FInterpTo(BrowRaiseIntensity, 0.4f * EmotionWeight, DeltaTime, 2.0f);
    }
    else // Neutral or other emotions
    {
        SmileIntensity = FMath::FInterpTo(SmileIntensity, 0.1f, DeltaTime, 1.5f);
        BrowRaiseIntensity = FMath::FInterpTo(BrowRaiseIntensity, 0.0f, DeltaTime, 1.5f);
    }
}

void UConciergAnimInstance::ApplyEmotionalModifiers()
{
    // Apply emotional intensity to all animation weights
    float IntensityMultiplier = EmotionIntensity;
    
    // Modify animation speeds and intensities based on emotion
    if (CurrentEmotion == "Excited")
    {
        // Faster, more energetic movements
        IntensityMultiplier *= 1.2f;
    }
    else if (CurrentEmotion == "Calm" || CurrentEmotion == "Relaxed")
    {
        // Slower, more relaxed movements
        IntensityMultiplier *= 0.8f;
    }
    
    // Apply to breathing
    BreathingIntensity *= IntensityMultiplier;
    
    // Apply to posture
    PostureWeight *= IntensityMultiplier;
}

FVector2D UConciergAnimInstance::CalculateEyeLookDirection(FVector WorldTarget)
{
    if (!OwnerPawn)
    {
        return FVector2D::ZeroVector;
    }
    
    // Get head location and rotation
    FVector HeadLocation = OwnerPawn->GetActorLocation();
    FRotator HeadRotation = OwnerPawn->GetActorRotation();
    
    // Calculate direction to target
    FVector DirectionToTarget = (WorldTarget - HeadLocation).GetSafeNormal();
    
    // Convert to local space relative to head rotation
    FVector LocalDirection = HeadRotation.UnrotateVector(DirectionToTarget);
    
    // Convert to eye look coordinates (X = horizontal, Y = vertical)
    FVector2D EyeDirection;
    EyeDirection.X = FMath::Clamp(LocalDirection.Y, -0.5f, 0.5f); // Left/Right
    EyeDirection.Y = FMath::Clamp(LocalDirection.Z, -0.3f, 0.3f); // Up/Down
    
    return EyeDirection;
}

float UConciergAnimInstance::GetEmotionWeight(const FString& EmotionName)
{
    // Calculate blend weight based on emotion transition
    float BlendAlpha = EmotionBlendTime / EmotionBlendDuration;
    BlendAlpha = FMath::Clamp(BlendAlpha, 0.0f, 1.0f);
    
    // Apply smooth curve to blend
    BlendAlpha = FMath::SmoothStep(0.0f, 1.0f, BlendAlpha);
    
    return BlendAlpha * EmotionIntensity;
}