# AWS Bedrock Nova Sonic Integration Guide

## Overview
Amazon Bedrock Nova Sonic provides advanced speech-to-speech capabilities, enabling natural voice interactions with your MetaHuman concierge.

## Setup Requirements

### 1. AWS Account Configuration
```bash
# Install AWS CLI
aws configure set aws_access_key_id YOUR_ACCESS_KEY
aws configure set aws_secret_access_key YOUR_SECRET_KEY
aws configure set default.region us-east-1
```

### 2. Bedrock Permissions
Required IAM permissions:
```json
{
    "Version": "2012-10-17",
    "Statement": [
        {
            "Effect": "Allow",
            "Action": [
                "bedrock:InvokeModel",
                "bedrock:InvokeModelWithResponseStream"
            ],
            "Resource": "arn:aws:bedrock:*::foundation-model/amazon.nova-sonic-v1:0"
        }
    ]
}
```

## Implementation Architecture

### 1. C++ Integration Class
```cpp
// BedrockAudioManager.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"
#include "aws/core/Aws.h"
#include "aws/bedrock-runtime/BedrockRuntimeClient.h"
#include "BedrockAudioManager.generated.h"

UCLASS()
class RESTAURANTCONCIERGE_API ABedrockAudioManager : public AActor
{
    GENERATED_BODY()

public:
    ABedrockAudioManager();

    UFUNCTION(BlueprintCallable, Category = "Bedrock")
    void ProcessSpeechInput(const TArray<uint8>& AudioData);

    UFUNCTION(BlueprintCallable, Category = "Bedrock")
    void SetRestaurantContext(const FString& Location, const TArray<FString>& Preferences);

protected:
    virtual void BeginPlay() override;

private:
    std::shared_ptr<Aws::BedrockRuntime::BedrockRuntimeClient> BedrockClient;
    FString CurrentLocation;
    TArray<FString> UserPreferences;
    
    void InitializeAWS();
    void ProcessBedrockResponse(const FString& Response);
    FString BuildRestaurantPrompt(const FString& UserInput);
};
```

### 2. Speech Processing Pipeline

#### Audio Input Processing
```cpp
void ABedrockAudioManager::ProcessSpeechInput(const TArray<uint8>& AudioData)
{
    // Convert audio to base64 for Bedrock
    FString AudioBase64 = FBase64::Encode(AudioData);
    
    // Prepare Bedrock request
    Aws::BedrockRuntime::Model::InvokeModelRequest Request;
    Request.SetModelId("amazon.nova-sonic-v1:0");
    
    // Build request body with restaurant context
    FString RequestBody = BuildSpeechRequest(AudioBase64);
    Request.SetBody(Aws::Utils::ByteBuffer((unsigned char*)TCHAR_TO_UTF8(*RequestBody), RequestBody.Len()));
    
    // Async call to Bedrock
    BedrockClient->InvokeModelAsync(Request, 
        [this](const auto& Client, const auto& Request, const auto& Outcome, const auto& Context)
        {
            HandleBedrockResponse(Outcome);
        });
}
```

#### Restaurant Context Integration
```cpp
FString ABedrockAudioManager::BuildRestaurantPrompt(const FString& UserInput)
{
    FString SystemPrompt = FString::Printf(TEXT(
        "You are a friendly restaurant concierge assistant. "
        "Current location: %s. "
        "User preferences: %s. "
        "Provide helpful restaurant recommendations with details about: "
        "- Operating hours "
        "- Cuisine type "
        "- Price range "
        "- Ratings "
        "- Special features "
        "Keep responses conversational and under 30 seconds when spoken."
    ), *CurrentLocation, *FString::Join(UserPreferences, TEXT(", ")));
    
    return SystemPrompt + TEXT("\n\nUser: ") + UserInput;
}
```

## Blueprint Integration

### 1. Audio Capture Blueprint
```
Event BeginPlay
├── Initialize Microphone
├── Set Audio Format (16kHz, 16-bit, Mono)
└── Bind Audio Input Events

On Voice Activity Detected
├── Start Recording
├── Buffer Audio Data
└── On Speech End → Process with Bedrock

On Bedrock Response
├── Parse Audio Response
├── Play Synthesized Speech
└── Trigger MetaHuman Lip Sync
```

### 2. Restaurant Data Integration
```
On Restaurant Query
├── Get Current Location
├── Call Restaurant APIs
├── Format Data for Bedrock Context
└── Update Conversation Context

On Restaurant Selection
├── Fetch Detailed Information
├── Prepare Visual Assets
└── Generate Detailed Response
```

## Configuration

### 1. Project Settings
```ini
[/Script/RestaurantConcierge.BedrockSettings]
ModelId=amazon.nova-sonic-v1:0
Region=us-east-1
MaxResponseTime=30.0
AudioSampleRate=16000
AudioChannels=1
```

### 2. Environment Variables
```bash
export AWS_REGION=us-east-1
export BEDROCK_MODEL_ID=amazon.nova-sonic-v1:0
export RESTAURANT_API_KEY=your_api_key
```

## Performance Optimization

### 1. Audio Processing
- Use 16kHz sample rate for optimal Bedrock performance
- Implement voice activity detection to reduce API calls
- Buffer audio in 3-5 second chunks for responsiveness

### 2. Response Caching
- Cache common restaurant queries locally
- Implement intelligent prefetching based on location
- Use streaming responses for longer conversations

### 3. Error Handling
```cpp
void HandleBedrockError(const Aws::Client::AWSError<Aws::BedrockRuntime::BedrockRuntimeErrors>& Error)
{
    switch(Error.GetErrorType())
    {
        case Aws::BedrockRuntime::BedrockRuntimeErrors::THROTTLING:
            // Implement exponential backoff
            break;
        case Aws::BedrockRuntime::BedrockRuntimeErrors::SERVICE_UNAVAILABLE:
            // Fallback to cached responses
            break;
        default:
            // Log error and provide user feedback
            break;
    }
}
```

## Testing Strategy

### 1. Unit Tests
- Audio format validation
- API request/response handling
- Error condition handling

### 2. Integration Tests
- End-to-end speech processing
- Restaurant data integration
- Performance benchmarks

### 3. User Testing
- Natural conversation flow
- Response accuracy
- Latency measurements