#include "EngineSoundVolumeLibrary.h"

TArray UEngineSoundVolumeLibrary::CalculateEngineSoundVolumes(
    float RPM,
    const TArray& EngineSounds)
{
    TArray Volumes;

    const int32 Count = EngineSounds.Num();

    if (Count <= 0)
    {
        return Volumes;
    }

    Volumes.Init(0.0f, Count);

    // Clamp RPM to non-negative values
    RPM = FMath::Max(0.0f, RPM);

    // If RPM is at or below the top threshold of the first entry (e.g. 0-1000 RPM)
    if (RPM <= EngineSounds[0].RPM)
    {
        Volumes[0] = 1.0f;
        return Volumes;
    }

    // Find the first entry whose RPM ceiling is >= the current RPM
    int32 UpperIndex = INDEX_NONE;

    for (int32 Index = 1; Index < Count; ++Index)
    {
        if (EngineSounds[Index].RPM >= RPM)
        {
            UpperIndex = Index;
            break;
        }
    }

    // RPM is above all defined RPM points (above the highest ceiling)
    if (UpperIndex == INDEX_NONE)
    {
        Volumes[Count - 1] = 1.0f;
        return Volumes;
    }

    const int32 LowerIndex = UpperIndex - 1;

    const float LowerRPM = EngineSounds[LowerIndex].RPM;
    const float UpperRPM = EngineSounds[UpperIndex].RPM;
    const float RPMRange = UpperRPM - LowerRPM;

    // Protect against duplicate or invalid RPM range values
    if (FMath::IsNearlyZero(RPMRange) || RPMRange < 0.0f)
    {
        Volumes[UpperIndex] = 1.0f;
        return Volumes;
    }

    const float LinearAlpha = FMath::Clamp(
        (RPM - LowerRPM) / RPMRange,
        0.0f,
        1.0f
    );

    // SmoothStep gives a softer crossfade than linear interpolation
    const float Alpha = FMath::SmoothStep(
        0.0f,
        1.0f,
        LinearAlpha
    );

    Volumes[LowerIndex] = 1.0f - Alpha;
    Volumes[UpperIndex] = Alpha;

    return Volumes;
}