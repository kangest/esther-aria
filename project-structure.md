# Detailed Project Structure

## 1. Unreal Engine Project Setup

### Core Directories
```
RestaurantConcierge/
├── RestaurantConcierge.uproject
├── Source/RestaurantConcierge/
│   ├── RestaurantConcierge.Build.cs
│   ├── RestaurantConciergePawn.h/cpp
│   ├── BedrockAudioManager.h/cpp
│   ├── RestaurantDataManager.h/cpp
│   └── LocationService.h/cpp
├── Content/
│   ├── MetaHuman/
│   │   ├── Concierge_Character/
│   │   ├── Animations/
│   │   └── Materials/
│   ├── UI/
│   │   ├── MainInterface/
│   │   ├── RestaurantCards/
│   │   └── LocationSelector/
│   ├── Audio/
│   │   ├── VoiceProcessing/
│   │   └── SoundEffects/
│   └── Maps/
│       ├── MainLevel.umap
│       └── TestingLevel.umap
└── Plugins/
    ├── BedrockIntegration/
    └── RestaurantAPI/
```

## 2. Key Components

### A. MetaHuman Character
- **Character Blueprint**: BP_RestaurantConcierge
- **Animation Blueprint**: ABP_Concierge
- **Facial Animation**: Linked to speech synthesis
- **Gesture System**: Context-aware hand movements

### B. AWS Bedrock Integration
- **Speech Input**: Microphone capture and processing
- **Nova Sonic API**: Speech-to-speech conversion
- **Response Processing**: Audio output and lip-sync
- **Error Handling**: Fallback mechanisms

### C. Restaurant Data System
- **Location Services**: GPS/manual location input
- **API Integration**: Google Places, Yelp, OpenTable
- **Data Caching**: Local storage for performance
- **Image Processing**: Restaurant and food photos

### D. User Interface
- **Voice Activation**: Push-to-talk or always listening
- **Visual Feedback**: Speech recognition indicators
- **Restaurant Display**: Cards with photos and details
- **Settings Panel**: Preferences and configuration

## 3. Technical Implementation

### Speech Processing Pipeline
1. **Audio Capture** → Microphone input
2. **Preprocessing** → Noise reduction, normalization
3. **Bedrock Nova Sonic** → Speech-to-speech processing
4. **Response Generation** → Restaurant-specific responses
5. **Audio Output** → Synthesized speech playback
6. **Lip Sync** → MetaHuman facial animation

### Restaurant Data Flow
1. **Location Detection** → Current or specified location
2. **API Queries** → Multiple restaurant data sources
3. **Data Aggregation** → Combine and normalize results
4. **Filtering** → Apply user preferences
5. **Presentation** → Display in UI with MetaHuman narration

### Conversation Management
- **Context Tracking**: Remember previous requests
- **Intent Recognition**: Understand user goals
- **Response Generation**: Natural, helpful responses
- **Error Recovery**: Handle misunderstandings gracefully