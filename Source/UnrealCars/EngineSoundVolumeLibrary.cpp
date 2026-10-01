#include "EngineSoundVolumeLibrary.h"

TArray<float> UEngineSoundVolumeLibrary::CalculateEngineSoundVolumes(
    float RPM,
    const TArray<FEngineSoundEntry>& EngineSounds)
{
    TArray<float> Volumes;
    const int32 Count = EngineSounds.Num();

    if (Count <= 0)
    {
        return Volumes;
    }

    Volumes.Init(0.0f, Count);

    // Ensure RPM is valid
    RPM = FMath::Max(0.0f, RPM);

    // Below or at the first threshold
    if (RPM <= EngineSounds[0].RPM)
    {
        Volumes[0] = 1.0f;
        return Volumes;
    }

    // Above or at the highest threshold
    if (RPM >= EngineSounds[Count - 1].RPM)
    {
        Volumes[Count - 1] = 1.0f;
        return Volumes;
    }

    // Find the matching RPM bucket
    for (int32 Index = 0; Index < Count - 1; ++Index)
    {
        const float LowerRPM = EngineSounds[Index].RPM;
        const float UpperRPM = EngineSounds[Index + 1].RPM;

        if (RPM >= LowerRPM && RPM <= UpperRPM)
        {
            const float RPMRange = UpperRPM - LowerRPM;

            if (FMath::IsNearlyZero(RPMRange))
            {
                Volumes[Index] = 1.0f;
                return Volumes;
            }

            // Normalized progress between 0.0 and 1.0
            const float LinearAlpha =
                FMath::Clamp((RPM - LowerRPM) / RPMRange, 0.0f, 1.0f);

            // Equal-Power Crossfade
            const float Angle = LinearAlpha * HALF_PI;

            Volumes[Index] = FMath::Cos(Angle);
            Volumes[Index + 1] = FMath::Sin(Angle);

            return Volumes;
        }
    }

    // Fallback safety net
    Volumes[0] = 1.0f;
    return Volumes;
}