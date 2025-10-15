#include "RestaurantConciergePawn.h"
#include "ConciergAnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ARestaurantConciergePawn::ARestaurantConciergePawn()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create root component
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));

    // Create MetaHuman skeletal mesh
    MetaHumanMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MetaHumanMesh"));
    MetaHumanMesh->SetupAttachment(RootComponent);

    // Create voice audio component
    VoiceAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("VoiceAudioComponent"));
    VoiceAudioComponent->SetupAttachment(MetaHumanMesh, TEXT("head")); // Attach to head bone

    // Create camera target for cinematics
    CameraTarget = CreateDefaultSubobject<USceneComponent>(TEXT("CameraTarget"));
    CameraTarget->SetupAttachment(MetaHumanMesh, TEXT("head"));
    CameraTarget->SetRelativeLocation(FVector(100.0f, 0.0f, 0.0f)); // In front of face

    // Initialize default values
    CurrentEmotion = "Neutral";
    CurrentEmotionIntensity = 1.0f;
    bIsSpeaking = false;
    bIsListening = false;
    LastIdleGestureTime = 0.0f;
    LastBlinkTime = 0.0f;
}

void ARestaurantConciergePawn::BeginPlay()
{
    Super::BeginPlay();

    // Get animation instance
    if (MetaHumanMesh && MetaHumanMesh->GetAnimInstance())
    {
        AnimInstance = Cast<UConciergAnimInstance>(MetaHumanMesh->GetAnimInstance());
        if (AnimInstance)
        {
            AnimInstance->SetOwnerPawn(this);
        }
    }

    // Initialize gesture and expression mappings
    InitializeGestures();
    InitializeFacialExpressions();

    // Bind audio component events
    if (VoiceAudioComponent)
    {
        VoiceAudioComponent->OnAudioFinished.AddDynamic(this, &ARestaurantConciergePawn::OnAudioFinished);
    }

    UE_LOG(LogTemp, Log, TEXT("RestaurantConciergePawn initialized"));
}

void ARestaurantConciergePawn::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    UpdateIdleBehavior(DeltaTime);
    UpdateEyeMovement(DeltaTime);
    UpdateBreathing(DeltaTime);
}

void ARestaurantConciergePawn::PlayGesture(const FString& GestureName)
{
    if (!MetaHumanMesh || !AnimInstance)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot play gesture: Missing mesh or animation instance"));
        return;
    }

    UAnimMontage** FoundGesture = GestureAnimations.Find(GestureName);
    if (FoundGesture && *FoundGesture)
    {
        float Duration = MetaHumanMesh->GetAnimInstance()->Montage_Play(*FoundGesture);
        
        // Bind completion callback
        FOnMontageEnded EndDelegate;
        EndDelegate.BindUObject(this, &ARestaurantConciergePawn::OnGestureAnimationComplete);
        MetaHumanMesh->GetAnimInstance()->Montage_SetEndDelegate(EndDelegate, *FoundGesture);

        UE_LOG(LogTemp, Log, TEXT("Playing gesture: %s (Duration: %.2f)"), *GestureName, Duration);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Gesture not found: %s"), *GestureName);
    }
}

void ARestaurantConciergePawn::StartSpeaking(USoundWave* AudioClip)
{
    if (!VoiceAudioComponent || !AudioClip)
    {
        UE_LOG(LogTemp, Warning, TEXT("Cannot start speaking: Missing audio component or clip"));
        return;
    }

    // Stop any current speech
    StopSpeaking();

    // Set new audio and play
    VoiceAudioComponent->SetSound(AudioClip);
    VoiceAudioComponent->Play();
    
    bIsSpeaking = true;

    // Update animation state
    if (AnimInstance)
    {
        AnimInstance->SetSpeakingState(true);
    }

    UE_LOG(LogTemp, Log, TEXT("Started speaking"));
}

void ARestaurantConciergePawn::StopSpeaking()
{
    if (VoiceAudioComponent && VoiceAudioComponent->IsPlaying())
    {
        VoiceAudioComponent->Stop();
    }

    bIsSpeaking = false;

    // Update animation state
    if (AnimInstance)
    {
        AnimInstance->SetSpeakingState(false);
    }

    UE_LOG(LogTemp, Log, TEXT("Stopped speaking"));
}

bool ARestaurantConciergePawn::IsSpeaking() const
{
    return bIsSpeaking && VoiceAudioComponent && VoiceAudioComponent->IsPlaying();
}

void ARestaurantConciergePawn::SetEmotionalState(const FString& Emotion, float Intensity)
{
    CurrentEmotion = Emotion;
    CurrentEmotionIntensity = FMath::Clamp(Intensity, 0.0f, 1.0f);

    // Update animation instance
    if (AnimInstance)
    {
        AnimInstance->SetEmotionalState(Emotion, Intensity);
    }

    // Adjust voice parameters if needed
    if (VoiceAudioComponent)
    {
        float PitchMultiplier = 1.0f;
        
        if (Emotion == "Excited" || Emotion == "Happy")
        {
            PitchMultiplier = 1.05f + (Intensity * 0.1f);
        }
        else if (Emotion == "Sad" || Emotion == "Sympathetic")
        {
            PitchMultiplier = 0.95f - (Intensity * 0.1f);
        }
        
        VoiceAudioComponent->SetPitchMultiplier(PitchMultiplier);
    }

    OnEmotionChanged.Broadcast(Emotion);
    
    UE_LOG(LogTemp, Log, TEXT("Emotional state changed to: %s (Intensity: %.2f)"), *Emotion, Intensity);
}

void ARestaurantConciergePawn::SetListeningState(bool bIsListeningNew)
{
    bIsListening = bIsListeningNew;

    // Update animation state
    if (AnimInstance)
    {
        AnimInstance->SetListeningState(bIsListening);
    }

    // Trigger appropriate facial expression
    if (bIsListening)
    {
        SetFacialExpression("Attentive", 0.8f);
    }
    else
    {
        SetFacialExpression("Neutral", 1.0f);
    }

    UE_LOG(LogTemp, Log, TEXT("Listening state: %s"), bIsListening ? TEXT("True") : TEXT("False"));
}

void ARestaurantConciergePawn::TriggerIdleGesture()
{
    if (bIsSpeaking || IdleGestures.Num() == 0)
    {
        return;
    }

    // Select random idle gesture
    int32 RandomIndex = FMath::RandRange(0, IdleGestures.Num() - 1);
    FString SelectedGesture = IdleGestures[RandomIndex];
    
    PlayGesture(SelectedGesture);
    LastIdleGestureTime = GetWorld()->GetTimeSeconds();
}

void ARestaurantConciergePawn::SetFacialExpression(const FString& Expression, float Intensity)
{
    if (!MetaHumanMesh || !AnimInstance)
    {
        return;
    }

    UAnimMontage** FoundExpression = FacialExpressions.Find(Expression);
    if (FoundExpression && *FoundExpression)
    {
        // Play facial expression with specified intensity
        MetaHumanMesh->GetAnimInstance()->Montage_Play(*FoundExpression, Intensity);
        
        UE_LOG(LogTemp, Log, TEXT("Playing facial expression: %s (Intensity: %.2f)"), *Expression, Intensity);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Facial expression not found: %s"), *Expression);
    }
}

void ARestaurantConciergePawn::SetEyeLookTarget(FVector WorldLocation)
{
    EyeLookTarget = WorldLocation;
    bHasEyeLookTarget = true;

    if (AnimInstance)
    {
        AnimInstance->SetEyeLookTarget(WorldLocation);
    }
}

void ARestaurantConciergePawn::ResetEyeLook()
{
    bHasEyeLookTarget = false;
    
    if (AnimInstance)
    {
        AnimInstance->ResetEyeLook();
    }
}

void ARestaurantConciergePawn::OnAudioFinished()
{
    bIsSpeaking = false;
    
    if (AnimInstance)
    {
        AnimInstance->SetSpeakingState(false);
    }

    UE_LOG(LogTemp, Log, TEXT("Audio playback finished"));
}

void ARestaurantConciergePawn::OnGestureAnimationComplete(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage)
    {
        // Find gesture name by montage
        for (const auto& GesturePair : GestureAnimations)
        {
            if (GesturePair.Value == Montage)
            {
                OnGestureComplete.Broadcast(GesturePair.Key);
                UE_LOG(LogTemp, Log, TEXT("Gesture completed: %s"), *GesturePair.Key);
                break;
            }
        }
    }
}

void ARestaurantConciergePawn::UpdateIdleBehavior(float DeltaTime)
{
    if (bIsSpeaking || bIsListening)
    {
        return;
    }

    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Trigger idle gestures periodically
    if (CurrentTime - LastIdleGestureTime > IdleGestureFrequency)
    {
        // Add some randomness to make it feel more natural
        float RandomDelay = FMath::RandRange(0.0f, 5.0f);
        if (CurrentTime - LastIdleGestureTime > IdleGestureFrequency + RandomDelay)
        {
            TriggerIdleGesture();
        }
    }
}

void ARestaurantConciergePawn::UpdateEyeMovement(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Handle blinking
    if (CurrentTime - LastBlinkTime > (1.0f / BlinkFrequency))
    {
        // Add some randomness to blink timing
        float RandomBlink = FMath::RandRange(0.8f, 1.2f);
        if (CurrentTime - LastBlinkTime > (1.0f / BlinkFrequency) * RandomBlink)
        {
            // Trigger blink animation
            if (AnimInstance)
            {
                AnimInstance->TriggerBlink();
            }
            LastBlinkTime = CurrentTime;
        }
    }

    // Update eye look target
    if (AnimInstance)
    {
        if (bHasEyeLookTarget)
        {
            AnimInstance->SetEyeLookTarget(EyeLookTarget);
        }
        else
        {
            // Natural eye movement when no specific target
            AnimInstance->UpdateNaturalEyeMovement(DeltaTime);
        }
    }
}

void ARestaurantConciergePawn::UpdateBreathing(float DeltaTime)
{
    if (AnimInstance)
    {
        AnimInstance->UpdateBreathing(DeltaTime);
    }
}

FString ARestaurantConciergePawn::SelectContextualGesture(const FString& SpeechText)
{
    FString LowerText = SpeechText.ToLower();
    
    // Simple keyword-based gesture selection
    if (LowerText.Contains("welcome") || LowerText.Contains("hello"))
    {
        return "Welcome";
    }
    else if (LowerText.Contains("over there") || LowerText.Contains("that way") || LowerText.Contains("direction"))
    {
        return "Pointing";
    }
    else if (LowerText.Contains("first") || LowerText.Contains("second") || LowerText.Contains("options"))
    {
        return "Counting";
    }
    else if (LowerText.Contains("explain") || LowerText.Contains("because") || LowerText.Contains("however"))
    {
        return "Explaining";
    }
    else
    {
        return "Neutral";
    }
}

void ARestaurantConciergePawn::InitializeGestures()
{
    // Initialize gesture mappings
    // These would be set up in Blueprint or loaded from data assets
    
    IdleGestures.Add("SubtleNod");
    IdleGestures.Add("HandAdjust");
    IdleGestures.Add("ShoulderShift");
    
    UE_LOG(LogTemp, Log, TEXT("Gestures initialized: %d idle gestures available"), IdleGestures.Num());
}

void ARestaurantConciergePawn::InitializeFacialExpressions()
{
    // Initialize facial expression mappings
    // These would be set up in Blueprint or loaded from data assets
    
    UE_LOG(LogTemp, Log, TEXT("Facial expressions initialized"));
}