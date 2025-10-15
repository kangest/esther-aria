#include "RestaurantConciergeGameMode.h"
#include "RestaurantDataManager.h"
#include "BedrockAudioManager.h"
#include "RestaurantConciergePawn.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

ARestaurantConciergeGameMode::ARestaurantConciergeGameMode()
{
    // Set default pawn class
    DefaultPawnClass = ARestaurantConciergePawn::StaticClass();
    
    // Initialize pointers
    RestaurantDataManager = nullptr;
    BedrockAudioManager = nullptr;
    ConciergePawn = nullptr;
}

void ARestaurantConciergeGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogTemp, Log, TEXT("RestaurantConciergeGameMode starting..."));
    
    if (bAutoStartSystems)
    {
        InitializeSystems();
    }
}

void ARestaurantConciergeGameMode::InitializeSystems()
{
    UE_LOG(LogTemp, Log, TEXT("Initializing restaurant concierge systems..."));
    
    // Load configuration first
    LoadConfiguration();
    
    // Spawn core actors
    SpawnCoreActors();
    
    // Connect systems together
    ConnectSystems();
    
    UE_LOG(LogTemp, Log, TEXT("Restaurant concierge systems initialized successfully"));
}

void ARestaurantConciergeGameMode::SpawnCoreActors()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to get world reference"));
        return;
    }
    
    // Spawn Restaurant Data Manager
    if (!RestaurantDataManager)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("RestaurantDataManager");
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        RestaurantDataManager = World->SpawnActor<ARestaurantDataManager>(
            ARestaurantDataManager::StaticClass(),
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            SpawnParams
        );
        
        if (RestaurantDataManager)
        {
            UE_LOG(LogTemp, Log, TEXT("RestaurantDataManager spawned successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn RestaurantDataManager"));
        }
    }
    
    // Spawn Bedrock Audio Manager
    if (!BedrockAudioManager)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.Name = TEXT("BedrockAudioManager");
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
        
        BedrockAudioManager = World->SpawnActor<ABedrockAudioManager>(
            ABedrockAudioManager::StaticClass(),
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            SpawnParams
        );
        
        if (BedrockAudioManager)
        {
            UE_LOG(LogTemp, Log, TEXT("BedrockAudioManager spawned successfully"));
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn BedrockAudioManager"));
        }
    }
    
    // Get the concierge pawn (should be spawned as default pawn)
    if (!ConciergePawn)
    {
        APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(World, 0);
        ConciergePawn = Cast<ARestaurantConciergePawn>(PlayerPawn);
        
        if (ConciergePawn)
        {
            UE_LOG(LogTemp, Log, TEXT("ConciergePawn reference obtained"));
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("ConciergePawn not found - may need to be spawned manually"));
        }
    }
}

void ARestaurantConciergeGameMode::ConnectSystems()
{
    if (!RestaurantDataManager || !BedrockAudioManager)
    {
        UE_LOG(LogTemp, Error, TEXT("Cannot connect systems - missing core components"));
        return;
    }
    
    SetupSystemBindings();
    
    UE_LOG(LogTemp, Log, TEXT("Systems connected successfully"));
}

void ARestaurantConciergeGameMode::SetupSystemBindings()
{
    // Bind Restaurant Data Manager events to Bedrock Audio Manager
    if (RestaurantDataManager && BedrockAudioManager)
    {
        RestaurantDataManager->OnRestaurantsFound.AddDynamic(this, &ARestaurantConciergeGameMode::OnRestaurantsFound);
        RestaurantDataManager->OnAPIError.AddDynamic(this, &ARestaurantConciergeGameMode::OnRestaurantAPIError);
    }
    
    // Bind Bedrock Audio Manager events to Concierge Pawn
    if (BedrockAudioManager && ConciergePawn)
    {
        BedrockAudioManager->OnSpeechProcessed.AddDynamic(this, &ARestaurantConciergeGameMode::OnSpeechProcessed);
        BedrockAudioManager->OnAudioResponseReady.AddDynamic(this, &ARestaurantConciergeGameMode::OnAudioResponseReady);
        BedrockAudioManager->OnBedrockError.AddDynamic(this, &ARestaurantConciergeGameMode::OnBedrockError);
    }
    
    // Set up initial context
    if (BedrockAudioManager)
    {
        // Set default location
        TArray<FRestaurantData> EmptyRestaurants;
        BedrockAudioManager->SetRestaurantContext(DefaultLocation, EmptyRestaurants);
        
        // Set default user preferences
        TArray<FString> DefaultPreferences;
        DefaultPreferences.Add("Good ratings");
        DefaultPreferences.Add("Reasonable prices");
        BedrockAudioManager->UpdateUserPreferences(DefaultPreferences);
    }
}

void ARestaurantConciergeGameMode::LoadConfiguration()
{
    // Load API keys and configuration from project settings or config files
    // This would typically read from a secure configuration system
    
    if (bUseMockData)
    {
        UE_LOG(LogTemp, Log, TEXT("Using mock data for development"));
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("Using live API data"));
        
        // In a real implementation, load API keys securely
        if (RestaurantDataManager)
        {
            // RestaurantDataManager->SetAPIKeys("YOUR_GOOGLE_PLACES_KEY", "YOUR_YELP_KEY");
        }
        
        if (BedrockAudioManager)
        {
            BedrockAudioManager->SetBedrockConfiguration("us-east-1", "amazon.nova-sonic-v1:0");
        }
    }
}

// Event handlers for system integration
UFUNCTION()
void ARestaurantConciergeGameMode::OnRestaurantsFound(const TArray<FRestaurantData>& Restaurants)
{
    UE_LOG(LogTemp, Log, TEXT("GameMode: Received %d restaurants"), Restaurants.Num());
    
    // Update Bedrock context with new restaurant data
    if (BedrockAudioManager)
    {
        BedrockAudioManager->SetRestaurantContext(DefaultLocation, Restaurants);
    }
    
    // Update concierge pawn emotional state
    if (ConciergePawn && Restaurants.Num() > 0)
    {
        ConciergePawn->SetEmotionalState("Happy", 0.8f);
    }
}

UFUNCTION()
void ARestaurantConciergeGameMode::OnRestaurantAPIError(const FString& APIName, const FString& ErrorMessage)
{
    UE_LOG(LogTemp, Warning, TEXT("GameMode: Restaurant API Error [%s]: %s"), *APIName, *ErrorMessage);
    
    // Update concierge pawn to show concern
    if (ConciergePawn)
    {
        ConciergePawn->SetEmotionalState("Concerned", 0.6f);
    }
}

UFUNCTION()
void ARestaurantConciergeGameMode::OnSpeechProcessed(const FString& ResponseText)
{
    UE_LOG(LogTemp, Log, TEXT("GameMode: Speech processed: %s"), *ResponseText);
    
    // Analyze response for contextual gestures
    if (ConciergePawn)
    {
        // Simple gesture selection based on response content
        FString LowerResponse = ResponseText.ToLower();
        
        if (LowerResponse.Contains("welcome") || LowerResponse.Contains("hello"))
        {
            ConciergePawn->PlayGesture("Welcome");
            ConciergePawn->SetEmotionalState("Happy", 0.9f);
        }
        else if (LowerResponse.Contains("recommend") || LowerResponse.Contains("suggest"))
        {
            ConciergePawn->PlayGesture("Explaining");
            ConciergePawn->SetEmotionalState("Excited", 0.7f);
        }
        else if (LowerResponse.Contains("sorry") || LowerResponse.Contains("unfortunately"))
        {
            ConciergePawn->SetEmotionalState("Sympathetic", 0.8f);
        }
        else
        {
            ConciergePawn->SetEmotionalState("Neutral", 1.0f);
        }
    }
}

UFUNCTION()
void ARestaurantConciergeGameMode::OnAudioResponseReady(USoundWave* AudioResponse)
{
    UE_LOG(LogTemp, Log, TEXT("GameMode: Audio response ready"));
    
    // Play audio through concierge pawn
    if (ConciergePawn && AudioResponse)
    {
        ConciergePawn->StartSpeaking(AudioResponse);
    }
}

UFUNCTION()
void ARestaurantConciergeGameMode::OnBedrockError(const FString& ErrorType, const FString& ErrorMessage)
{
    UE_LOG(LogTemp, Error, TEXT("GameMode: Bedrock Error [%s]: %s"), *ErrorType, *ErrorMessage);
    
    // Update concierge pawn to show technical difficulty
    if (ConciergePawn)
    {
        ConciergePawn->SetEmotionalState("Concerned", 0.9f);
    }
}