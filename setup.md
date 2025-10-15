# Restaurant Concierge MetaHuman - Setup Guide

## Prerequisites

### Software Requirements
1. **Unreal Engine 5.3 or later**
   - Download from Epic Games Launcher
   - Ensure MetaHuman plugin is available

2. **Visual Studio 2022** (Windows) or **Xcode** (macOS)
   - Required for C++ compilation
   - Install with C++ development tools

3. **Git with LFS**
   - For version control of large assets
   - Configure LFS for .uasset, .umap files

### AWS Setup (for Production)
1. **AWS Account** with Bedrock access
2. **IAM Role** with Bedrock permissions
3. **API Keys** for restaurant data sources

## Quick Start (Development Mode)

### 1. Project Setup
```bash
# Clone or download the project
# Navigate to project directory
cd RestaurantConcierge

# Generate project files (Windows)
# Right-click RestaurantConcierge.uproject -> "Generate Visual Studio project files"

# Or use command line (if UE5 is in PATH)
# UnrealBuildTool -projectfiles -project="RestaurantConcierge.uproject" -game -rocket -progress
```

### 2. Initial Build
```bash
# Open RestaurantConcierge.sln in Visual Studio
# Build Solution (Ctrl+Shift+B)
# Or build from command line:
# MSBuild RestaurantConcierge.sln /p:Configuration=Development
```

### 3. First Launch
1. Open `RestaurantConcierge.uproject` in Unreal Engine
2. Allow compilation when prompted
3. The project will start in development mode with mock data

## MetaHuman Integration

### 1. Create Your Concierge Character
1. Open [MetaHuman Creator](https://metahuman.unrealengine.com/)
2. Design your restaurant concierge character
3. Export as Unreal Engine asset
4. Import to `Content/MetaHuman/` folder

### 2. Configure Character Blueprint
1. Create Blueprint based on `ARestaurantConciergePawn`
2. Set MetaHuman mesh as skeletal mesh component
3. Configure animation blueprint
4. Set as default pawn in GameMode

## API Configuration

### Development Mode (Mock Data)
- No API keys required
- Uses simulated restaurant data
- Mock Bedrock responses for testing

### Production Mode
1. **Google Places API**
   ```
   Get API key from Google Cloud Console
   Enable Places API
   Set daily quotas and billing
   ```

2. **Yelp Fusion API**
   ```
   Register at Yelp Developers
   Create app and get API key
   Note rate limits (5000/day free tier)
   ```

3. **AWS Bedrock**
   ```
   Configure AWS credentials
   Request access to Nova Sonic model
   Set up IAM permissions
   ```

### Setting API Keys
```cpp
// In Blueprint or C++
RestaurantDataManager->SetAPIKeys("YOUR_GOOGLE_KEY", "YOUR_YELP_KEY");
BedrockAudioManager->SetBedrockConfiguration("us-east-1", "amazon.nova-sonic-v1:0");
```

## Project Structure

```
RestaurantConcierge/
├── Content/
│   ├── MetaHuman/          # MetaHuman character assets
│   ├── UI/                 # User interface blueprints
│   ├── Audio/              # Audio assets and processing
│   ├── Maps/               # Level maps
│   └── Blueprints/         # Game logic blueprints
├── Source/
│   └── RestaurantConcierge/
│       ├── RestaurantData.h/cpp           # Data structures
│       ├── RestaurantDataManager.h/cpp   # API integration
│       ├── BedrockAudioManager.h/cpp     # Speech processing
│       ├── RestaurantConciergePawn.h/cpp # MetaHuman controller
│       └── ConciergAnimInstance.h/cpp    # Animation logic
└── Config/                 # Project configuration
```

## Development Workflow

### 1. Core Systems Testing
```cpp
// Test restaurant data retrieval
FVector2D SeattleLocation(47.6062, -122.3321);
FSearchFilters Filters;
RestaurantDataManager->SearchRestaurants(SeattleLocation, Filters);

// Test speech processing
BedrockAudioManager->ProcessTextInput("Find me Italian restaurants");

// Test character animation
ConciergePawn->SetEmotionalState("Happy", 0.8f);
ConciergePawn->PlayGesture("Welcome");
```

### 2. Blueprint Integration
1. Create Blueprint classes inheriting from C++ classes
2. Configure properties in Blueprint editor
3. Set up event bindings for system communication
4. Test in PIE (Play in Editor)

### 3. UI Development
1. Create UMG widgets for restaurant display
2. Bind to restaurant data events
3. Implement voice activation controls
4. Add settings and preferences panels

## Troubleshooting

### Common Issues

**Build Errors:**
- Ensure all dependencies are installed
- Check Unreal Engine version compatibility
- Verify Visual Studio C++ tools

**MetaHuman Issues:**
- Confirm MetaHuman plugin is enabled
- Check skeletal mesh import settings
- Verify animation blueprint setup

**API Connection Issues:**
- Validate API keys and quotas
- Check network connectivity
- Review AWS credentials and permissions

**Audio Issues:**
- Verify microphone permissions
- Check audio device settings
- Test with mock audio data first

### Debug Logging
Enable verbose logging in `DefaultEngine.ini`:
```ini
[Core.Log]
LogTemp=Verbose
LogRestaurant=Verbose
LogBedrock=Verbose
LogMetaHuman=Verbose
```

## Next Steps

1. **Complete MetaHuman Setup**
   - Import your custom character
   - Configure facial animations
   - Set up lip sync system

2. **Implement UI**
   - Restaurant browsing interface
   - Voice activation controls
   - Settings and preferences

3. **Production Configuration**
   - Set up real API keys
   - Configure AWS Bedrock
   - Implement security measures

4. **Testing and Polish**
   - User experience testing
   - Performance optimization
   - Bug fixes and refinements

## Support

- Check documentation in `/Documentation/` folder
- Review example implementations in Blueprints
- Test with mock data before using live APIs
- Monitor logs for debugging information

Happy building! 🚀