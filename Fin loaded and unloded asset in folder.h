// Load asset registry module
FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(FName("AssetRegistry"));
IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

// Scan specific path
TArray<FString> PathsToScan;
PathsToScan.Add(TEXT("/Game/Story/"));
AssetRegistry.ScanPathsSynchronous(PathsToScan);

// Get all assets in the path, does not load them
TArray<FAssetData> ScriptAssetList;
AssetRegistry.GetAssetsByPath(FName("/Game/Story/"), ScriptAssetList, /*bRecursive=*/true);

// Ensure all assets are loaded and store their class
TArray<TSubclassOf<UScriptedWorldEvent>> EventClasses;
for (const FAssetData& Asset : ScriptAssetList)
{
	// Skip non blueprint assets
	const UBlueprint* BlueprintObj = Cast<UBlueprint>(Asset.GetAsset());
	if (!BlueprintObj)
		continue;

	// Check whether blueprint class has parent class we're looking for
	UClass* BlueprintClass = BlueprintObj->GeneratedClass;
	if (!BlueprintClass || !BlueprintClass->IsChildOf(UScriptedWorldEvent::StaticClass()))
		continue;

	// Store class
	EventClasses.Add(BlueprintClass);
}