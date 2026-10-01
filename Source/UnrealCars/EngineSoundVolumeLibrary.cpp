#include "EngineSoundVolumeLibrary.h"

TArray UEngineSoundVolumeLibrary::CalculateEngineSoundVolumes(
    float RPM,
    const TArray& EngineSounds) // English comment: Main volume calculation logic with Equal Power Crossfade
{
    TArray Volumes;
    const int32 Count = EngineSounds.Num();

    if (Count <= 0)
    {
        return Volumes;
    }

    Volumes.Init(0.0f, Count);

    // English comment: Ensure RPM is valid
    RPM = FMath::Max(0.0f, RPM);

    // English comment: Below or at the first threshold
    if (RPM <= EngineSounds[0].RPM)
    {
        Volumes[0] = 1.0f;
        return Volumes;
    }

    // English comment: Above or at the highest threshold
    if (RPM >= EngineSounds[Count - 1].RPM)
    {
        Volumes[Count - 1] = 1.0f;
        return Volumes;
    }

    // English comment: Find the matching RPM bucket
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

            // English comment: Normalized progress between 0.0 and 1.0
            const float LinearAlpha = FMath::Clamp((RPM - LowerRPM) / RPMRange, 0.0f, 1.0f);

            // English comment: Equal-Power Crossfade using sine/cosine or square root
            // Prevents loudness dipping in the middle of the transition range
            const float Angle = LinearAlpha * HALF_PI; // 0 to PI/2

            Volumes[Index] = FMath::Cos(Angle);     // Fades out from 1.0 to 0.0
            Volumes[Index + 1] = FMath::Sin(Angle); // Fades in from 0.0 to 1.0

            return Volumes;
        }
    }

    // English comment: Fallback safety net
    Volumes[0] = 1.0f;
    return Volumes;
}