#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EngineSoundVolumeLibrary.h"
#include "EngineSoundPitchLibrary.generated.h"

/**
 * Blueprint helper functions for engine sound pitch blending.
 */
UCLASS()
class UEngineSoundPitchLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    /**
     * Calculates one pitch multiplier for every Engine Sound Entry.
     *
     * The pitch multiplier is based on the RPM position between two
     * neighboring engine sound entries.
     *
     * For each active sound:
     *
     * Lower RPM entry:
     *   Starts at MinPitchMultiplier.
     *   Reaches 1.0 at the midpoint.
     *   Ends at 1.0.
     *
     * Upper RPM entry:
     *   Starts at 1.0.
     *   Reaches 1.0 at the midpoint.
     *   Ends at MaxPitchMultiplier.
     *
     * Example with Min = 0.9 and Max = 1.1:
     *
     *   At lower RPM:
     *     [0.9, 1.0, 0.0]
     *
     *   At midpoint:
     *     [1.0, 1.0, 0.0]
     *
     *   At upper RPM:
     *     [1.0, 1.1, 0.0]
     *
     * The returned array always has the same number of elements
     * as EngineSounds.
     */
    UFUNCTION(BlueprintPure, Category = "Audio|Engine Sound")
    static TArray<float> CalculateEngineSoundPitches(
        float RPM,
        const TArray<FEngineSoundEntry>& EngineSounds,
        float MinPitchMultiplier = 0.9f,
        float MaxPitchMultiplier = 1.1f
    );
};