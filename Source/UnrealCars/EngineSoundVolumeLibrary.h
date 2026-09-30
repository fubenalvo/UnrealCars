#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Sound/SoundWave.h"
#include "EngineSoundVolumeLibrary.generated.h"

/**
 * One engine sound sample.
 *
 * RPM is the RPM at which this sound should be at maximum volume.
 * Sound is the Sound Wave used by the corresponding Audio Component.
 */
USTRUCT(BlueprintType)
struct FEngineSoundEntry
{
    GENERATED_BODY()

    /** RPM at which this sound reaches maximum volume. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Sound")
    float RPM = 0.0f;

    /** Sound Wave associated with this RPM. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine Sound")
    TObjectPtr<USoundWave> Sound = nullptr;
};

/**
 * Blueprint helper functions for engine sound volume blending.
 */
UCLASS()
class UEngineSoundVolumeLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /**
     * Calculates one volume value for every Engine Sound Entry.
     *
     * Between two neighboring RPM points, their volumes are crossfaded.
     * At an entry's exact RPM, that entry has volume 1.0.
     *
     * The returned array always has the same number of elements as EngineSounds.
     *
     * Example:
     *   1000 RPM -> [1, 0, 0]
     *   1500 RPM -> [0.5, 0.5, 0]
     *   2000 RPM -> [0, 1, 0]
     *
     * SmoothStep is used for the crossfade so the transition is softer
     * than a simple linear interpolation.
     */
    UFUNCTION(BlueprintPure, Category = "Audio|Engine Sound")
    static TArray<float> CalculateEngineSoundVolumes(
        float RPM,
        const TArray<FEngineSoundEntry>& EngineSounds
    );
};
