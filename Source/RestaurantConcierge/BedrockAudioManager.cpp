#include "BedrockAudioManager.h"
#include "Http.h"
#include "Json.h"
#include "Misc/Base64.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

ABedrockAudioManager::ABedrockAudioManager()
{
    PrimaryActorTick.bCanEverTick = true;

    // Create audio output component
    AudioOutputComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioOutputComponent"));
    RootComponent = AudioOutputComponent;
}

void ABedrockAudioManager::BeginPlay()
{
    Super::BeginPlay();
    
    InitializeAudioCapture();
    
    UE_LOG(LogTemp, Log, TEXT("BedrockAudioManager initialized"));
}

void ABedrockAudioManager::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    
    if (bIsListening)
    {
        // Check for recording timeout
        float CurrentTime = GetWorld()->GetTimeSeconds();
        if (CurrentTime - RecordingStartTime > MaxRecordingDuration)
        {
            UE_LOG(LogTemp, Warning, TEXT("Recording timeout reached"));
            StopListening();
        }
        
        // Voice activity detection would go here
        // For now, we'll use a simple timer-based approach
    }
}

void ABedrockAudioManager::StartListening()
{
    if (bIsListening || bIsProcessing)
    {
        return;
    }
    
    bIsListening = true;
    RecordingStartTime = GetWorld()->GetTimeSeconds();
    ResetAudioBuffer();
    
    UE_LOG(LogTemp, Log, TEXT("Started listening for speech input"));
    
    // In a real implementation, this would start audio capture from microphone
    // For development, we'll simulate this with a timer
    if (bUseMockBedrock)
    {
        FTimerHandle TimerHandle;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this]()
        {
            // Simulate receiving audio input after 3 seconds
            ProcessTextInput("I'm looking for a good Italian restaurant nearby");
        }, 3.0f, false);
    }
}

void ABedrockAudioManager::StopListening()
{
    if (!bIsListening)
    {
        return;
    }
    
    bIsListening = false;
    
    UE_LOG(LogTemp, Log, TEXT("Stopped listening, processing audio..."));
    
    // Process the captured audio
    if (AudioBuffer.Num() > 0)
    {
        ProcessSpeechInput(AudioBuffer);
    }
}

void ABedrockAudioManager::ProcessSpeechInput(const TArray<uint8>& AudioData)
{
    if (bIsProcessing)
    {
        UE_LOG(LogTemp, Warning, TEXT("Already processing speech input"));
        return;
    }
    
    bIsProcessing = true;
    
    if (bUseMockBedrock)
    {
        // For development, simulate speech-to-text conversion
        ProcessTextInput("I want to find a good restaurant for dinner tonight");
        return;
    }
    
    // Convert audio to required format
    TArray<uint8> ProcessedAudio = ConvertAudioToFormat(AudioData);
    FString AudioBase64 = EncodeAudioToBase64(ProcessedAudio);
    
    // Build and send Bedrock request
    FString RequestBody = BuildBedrockRequestBody("", AudioBase64);
    SendBedrockRequest(RequestBody);
}

void ABedrockAudioManager::ProcessTextInput(const FString& InputText)
{
    if (bIsProcessing)
    {
        UE_LOG(LogTemp, Warning, TEXT("Already processing input"));
        return;
    }
    
    bIsProcessing = true;
    
    UE_LOG(LogTemp, Log, TEXT("Processing text input: %s"), *InputText);
    
    if (bUseMockBedrock)
    {
        ProcessMockBedrock(InputText);
        return;
    }
    
    // Build and send Bedrock request
    FString RequestBody = BuildBedrockRequestBody(InputText);
    SendBedrockRequest(RequestBody);
}

void ABedrockAudioManager::ProcessMockBedrock(const FString& InputText)
{
    // Simulate processing delay
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, InputText]()
    {
        FString MockResponse = GenerateMockResponse(InputText);
        
        // Broadcast text response
        OnSpeechProcessed.Broadcast(MockResponse);
        
        // For now, we don't generate actual audio in mock mode
        // In a real implementation, this would be synthesized speech
        
        bIsProcessing = false;
        
        UE_LOG(LogTemp, Log, TEXT("Mock Bedrock response: %s"), *MockResponse);
        
    }, 2.0f, false); // Simulate 2-second processing time
}

FString ABedrockAudioManager::GenerateMockResponse(const FString& InputText)
{
    // Simple mock response generation based on input keywords
    FString LowerInput = InputText.ToLower();
    
    if (LowerInput.Contains("italian"))
    {
        if (CurrentRestaurants.Num() > 0)
        {
            return FString::Printf(TEXT("I found several great Italian restaurants nearby! The top recommendation is %s, which has a 4.5-star rating and serves authentic Italian cuisine. Would you like to hear more details about this restaurant or see other options?"), 
                *CurrentRestaurants[0].Name);
        }
        else
        {
            return TEXT("I'd be happy to help you find Italian restaurants! Let me search for Italian restaurants in your area. One moment please...");
        }
    }
    else if (LowerInput.Contains("restaurant") || LowerInput.Contains("food") || LowerInput.Contains("eat"))
    {
        if (CurrentRestaurants.Num() > 0)
        {
            return FString::Printf(TEXT("I have information about %d restaurants in your area. What type of cuisine are you in the mood for today? I can recommend options based on Italian, Asian, American, or other cuisines."), 
                CurrentRestaurants.Num());
        }
        else
        {
            return TEXT("I'd be delighted to help you find a great restaurant! What type of cuisine are you interested in, and do you have any preferences for price range or distance?");
        }
    }
    else if (LowerInput.Contains("hello") || LowerInput.Contains("hi"))
    {
        return TEXT("Hello! I'm your restaurant concierge assistant. I'm here to help you discover amazing dining experiences in your area. What kind of restaurant are you looking for today?");
    }
    else if (LowerInput.Contains("hours") || LowerInput.Contains("open"))
    {
        if (CurrentRestaurants.Num() > 0)
        {
            return FString::Printf(TEXT("Let me check the operating hours for you. %s is currently open and serves until 10 PM tonight. Would you like me to check the hours for other restaurants as well?"), 
                *CurrentRestaurants[0].Name);
        }
        else
        {
            return TEXT("I can help you check restaurant hours! Which restaurant would you like to know about?");
        }
    }
    else
    {
        return TEXT("I understand you're looking for restaurant information. Could you tell me more specifically what you'd like to know? I can help with finding restaurants by cuisine type, checking hours, reading reviews, or getting directions.");
    }
}

void ABedrockAudioManager::SetRestaurantContext(const FString& Location, const TArray<FRestaurantData>& Restaurants)
{
    CurrentLocation = Location;
    CurrentRestaurants = Restaurants;
    
    // Build context string for Bedrock
    RestaurantContext = TEXT("Current location: ") + Location + TEXT("\n\n");
    
    if (Restaurants.Num() > 0)
    {
        RestaurantContext += TEXT("Available restaurants:\n");
        
        for (int32 i = 0; i < FMath::Min(Restaurants.Num(), 10); i++)
        {
            const FRestaurantData& Restaurant = Restaurants[i];
            RestaurantContext += FString::Printf(TEXT("%d. %s - %s cuisine, %s price range, %.1f stars\n"), 
                i + 1, *Restaurant.Name, 
                Restaurant.CuisineTypes.Num() > 0 ? *Restaurant.CuisineTypes[0] : TEXT("Various"),
                *Restaurant.PriceLevel, Restaurant.Rating);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Restaurant context updated: %d restaurants in %s"), Restaurants.Num(), *Location);
}

void ABedrockAudioManager::UpdateUserPreferences(const TArray<FString>& Preferences)
{
    UserPreferences = Preferences;
    UE_LOG(LogTemp, Log, TEXT("User preferences updated: %s"), *FString::Join(Preferences, TEXT(", ")));
}

void ABedrockAudioManager::SetBedrockConfiguration(const FString& Region, const FString& ModelId)
{
    BedrockRegion = Region;
    BedrockModelId = ModelId;
    UE_LOG(LogTemp, Log, TEXT("Bedrock configuration updated: %s in %s"), *ModelId, *Region);
}

FString ABedrockAudioManager::BuildBedrockRequestBody(const FString& InputText, const FString& AudioBase64)
{
    TSharedPtr<FJsonObject> RequestObject = MakeShareable(new FJsonObject);
    
    // Model configuration
    RequestObject->SetStringField(TEXT("modelId"), BedrockModelId);
    
    // System prompt
    FString SystemPrompt = BuildSystemPrompt();
    RequestObject->SetStringField(TEXT("systemPrompt"), SystemPrompt);
    
    // Input data
    if (!InputText.IsEmpty())
    {
        RequestObject->SetStringField(TEXT("inputText"), InputText);
    }
    
    if (!AudioBase64.IsEmpty())
    {
        RequestObject->SetStringField(TEXT("inputAudio"), AudioBase64);
    }
    
    // Response configuration
    TSharedPtr<FJsonObject> ResponseConfig = MakeShareable(new FJsonObject);
    ResponseConfig->SetBoolField(TEXT("includeAudio"), true);
    ResponseConfig->SetBoolField(TEXT("includeText"), true);
    RequestObject->SetObjectField(TEXT("responseConfig"), ResponseConfig);
    
    FString OutputString;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
    FJsonSerializer::Serialize(RequestObject.ToSharedRef(), Writer);
    
    return OutputString;
}

FString ABedrockAudioManager::BuildSystemPrompt()
{
    FString SystemPrompt = TEXT("You are a friendly and knowledgeable restaurant concierge assistant. ");
    SystemPrompt += TEXT("Your role is to help users discover great dining experiences by providing personalized restaurant recommendations. ");
    
    SystemPrompt += TEXT("Guidelines:\n");
    SystemPrompt += TEXT("- Be conversational, warm, and enthusiastic about food and dining\n");
    SystemPrompt += TEXT("- Provide specific details about restaurants including cuisine type, price range, ratings, and hours\n");
    SystemPrompt += TEXT("- Ask clarifying questions to better understand user preferences\n");
    SystemPrompt += TEXT("- Keep responses under 30 seconds when spoken\n");
    SystemPrompt += TEXT("- If you don't have specific information, acknowledge it and offer to help in other ways\n\n");
    
    if (!CurrentLocation.IsEmpty())
    {
        SystemPrompt += TEXT("Current location: ") + CurrentLocation + TEXT("\n");
    }
    
    if (UserPreferences.Num() > 0)
    {
        SystemPrompt += TEXT("User preferences: ") + FString::Join(UserPreferences, TEXT(", ")) + TEXT("\n");
    }
    
    if (!RestaurantContext.IsEmpty())
    {
        SystemPrompt += TEXT("\n") + RestaurantContext;
    }
    
    return SystemPrompt;
}

void ABedrockAudioManager::SendBedrockRequest(const FString& RequestBody)
{
    // In a real implementation, this would send to AWS Bedrock
    // For now, we'll simulate the request
    
    UE_LOG(LogTemp, Log, TEXT("Sending Bedrock request (simulated)"));
    
    if (bUseMockBedrock)
    {
        // Already handled in ProcessTextInput/ProcessSpeechInput
        return;
    }
    
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    Request->OnProcessRequestComplete().BindUObject(this, &ABedrockAudioManager::OnBedrockResponse);
    
    // AWS Bedrock endpoint (would need proper AWS SDK integration)
    FString URL = FString::Printf(TEXT("https://bedrock-runtime.%s.amazonaws.com/model/%s/invoke"), 
        *BedrockRegion, *BedrockModelId);
    
    Request->SetURL(URL);
    Request->SetVerb("POST");
    Request->SetHeader("Content-Type", "application/json");
    Request->SetHeader("Authorization", "AWS4-HMAC-SHA256 ..."); // Would need proper AWS signing
    Request->SetContentAsString(RequestBody);
    
    Request->ProcessRequest();
}

void ABedrockAudioManager::OnBedrockResponse(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
    bIsProcessing = false;
    
    if (!bWasSuccessful || !Response.IsValid())
    {
        HandleBedrockError("Network", "Failed to connect to Bedrock service");
        return;
    }
    
    if (Response->GetResponseCode() != 200)
    {
        FString ErrorMsg = FString::Printf(TEXT("HTTP %d: %s"), 
            Response->GetResponseCode(), *Response->GetContentAsString());
        HandleBedrockError("HTTP", ErrorMsg);
        return;
    }
    
    ProcessBedrockResponse(Response->GetContentAsString());
}

void ABedrockAudioManager::ProcessBedrockResponse(const FString& ResponseBody)
{
    TSharedPtr<FJsonObject> JsonObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(ResponseBody);
    
    if (!FJsonSerializer::Deserialize(Reader, JsonObject))
    {
        HandleBedrockError("Parse", "Failed to parse Bedrock response");
        return;
    }
    
    // Extract text response
    FString ResponseText;
    if (JsonObject->TryGetStringField(TEXT("outputText"), ResponseText))
    {
        OnSpeechProcessed.Broadcast(ResponseText);
    }
    
    // Extract audio response
    FString AudioBase64;
    if (JsonObject->TryGetStringField(TEXT("outputAudio"), AudioBase64))
    {
        USoundWave* AudioResponse = DecodeAudioFromBase64(AudioBase64);
        if (AudioResponse)
        {
            OnAudioResponseReady.Broadcast(AudioResponse);
        }
    }
    
    UE_LOG(LogTemp, Log, TEXT("Bedrock response processed successfully"));
}

void ABedrockAudioManager::HandleBedrockError(const FString& ErrorType, const FString& ErrorMessage)
{
    UE_LOG(LogTemp, Error, TEXT("Bedrock Error [%s]: %s"), *ErrorType, *ErrorMessage);
    OnBedrockError.Broadcast(ErrorType, ErrorMessage);
}

TArray<uint8> ABedrockAudioManager::ConvertAudioToFormat(const TArray<uint8>& InputAudio)
{
    // Convert audio to 16kHz, 16-bit, mono format required by Bedrock
    // This is a simplified implementation - real audio conversion would be more complex
    return InputAudio;
}

FString ABedrockAudioManager::EncodeAudioToBase64(const TArray<uint8>& AudioData)
{
    return FBase64::Encode(AudioData);
}

USoundWave* ABedrockAudioManager::DecodeAudioFromBase64(const FString& Base64Audio)
{
    TArray<uint8> AudioData;
    if (!FBase64::Decode(Base64Audio, AudioData))
    {
        return nullptr;
    }
    
    // Create a new SoundWave object
    USoundWave* SoundWave = NewObject<USoundWave>();
    if (!SoundWave)
    {
        return nullptr;
    }
    
    // Set up the sound wave properties
    SoundWave->SetSampleRate(SampleRate);
    SoundWave->NumChannels = Channels;
    SoundWave->Duration = static_cast<float>(AudioData.Num()) / (SampleRate * Channels * 2); // 16-bit = 2 bytes per sample
    
    // This would need proper audio format handling in a real implementation
    // For now, we'll return the SoundWave object without actual audio data
    
    return SoundWave;
}

void ABedrockAudioManager::InitializeAudioCapture()
{
    // Initialize audio capture system
    // This would set up microphone input in a real implementation
    UE_LOG(LogTemp, Log, TEXT("Audio capture initialized"));
}

void ABedrockAudioManager::CleanupAudioCapture()
{
    // Clean up audio capture resources
    UE_LOG(LogTemp, Log, TEXT("Audio capture cleaned up"));
}

void ABedrockAudioManager::ResetAudioBuffer()
{
    AudioBuffer.Empty();
    SilenceDuration = 0.0f;
}