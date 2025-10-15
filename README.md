# Restaurant Concierge Metahuman

A sophisticated AI-powered restaurant concierge built with Unreal Engine 5, featuring a photorealistic MetaHuman character that provides personalized restaurant recommendations using speech-to-speech interactions powered by Amazon Bedrock Nova Sonic.

## Features

- **Photorealistic MetaHuman Interface**: Interactive 3D character with natural animations
- **Speech-to-Speech AI**: Powered by Amazon Bedrock Nova Sonic for natural conversations
- **Location-Based Restaurant Discovery**: Find restaurants in current or specified locations
- **Comprehensive Restaurant Data**: Hours, cuisine types, pricing, ratings, photos
- **Real-time Interaction**: Natural conversation flow with contextual responses

## Architecture

### Core Components
1. **Unreal Engine 5 Frontend**: MetaHuman character and UI
2. **AWS Bedrock Integration**: Nova Sonic for speech processing
3. **Restaurant Data Service**: Location and restaurant information APIs
4. **Audio Processing Pipeline**: Speech input/output handling
5. **State Management**: Conversation context and user preferences

### Technology Stack
- Unreal Engine 5.3+
- MetaHuman Creator
- Amazon Bedrock (Nova Sonic)
- AWS SDK for C++
- Restaurant APIs (Google Places, Yelp, etc.)
- Audio processing libraries

## Project Structure

```
RestaurantConcierge/
├── Source/                     # Unreal Engine C++ source
├── Content/                    # Unreal Engine assets
│   ├── MetaHuman/             # MetaHuman character assets
│   ├── UI/                    # User interface blueprints
│   └── Audio/                 # Audio assets and processing
├── Plugins/                   # Custom plugins
│   ├── BedrockIntegration/    # AWS Bedrock plugin
│   └── RestaurantAPI/         # Restaurant data plugin
├── Config/                    # Configuration files
└── Documentation/             # Project documentation
```

## Getting Started

1. Set up Unreal Engine 5.3+
2. Configure AWS credentials for Bedrock access
3. Install required plugins and dependencies
4. Import MetaHuman character
5. Configure restaurant data APIs
6. Build and deploy

## Requirements

- Unreal Engine 5.3 or later
- AWS account with Bedrock access
- MetaHuman Creator account
- Restaurant API keys (Google Places, Yelp)
- Windows 10/11 or macOS for development