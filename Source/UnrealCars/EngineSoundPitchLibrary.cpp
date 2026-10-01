#include "EngineSoundPitchLibrary.h"

TArray<float> UEngineSoundPitchLibrary::CalculateEngineSoundPitches(
    float RPM,
    const TArray<FEngineSoundEntry>& EngineSounds,
    float MinPitchMultiplier,
    float MaxPitchMultiplier)
{
    TArray<float> Pitches;

    const int32 Count = EngineSounds.Num();

    if (Count <= 0)
    {
        return Pitches;
    }

    Pitches.Init(1.0f, Count);

    // English comment: Clamp the pitch range to prevent invalid multiplier values
    MinPitchMultiplier = FMath::Max(0.0f, MinPitchMultiplier);
    MaxPitchMultiplier = FMath::Max(MinPitchMultiplier, MaxPitchMultiplier);

    // English comment: Ensure RPM is valid
    RPM = FMath::Max(0.0f, RPM);

    // English comment: Below or at the first threshold
    if (RPM <= EngineSounds[0].RPM)
    {
        Pitches[0] = MinPitchMultiplier;
        return Pitches;
    }

    // English comment: Above or at the highest threshold
    if (RPM >= EngineSounds[Count - 1].RPM)
    {
        Pitches[Count - 1] = MaxPitchMultiplier;
        return Pitches;
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
                Pitches[Index] = 1.0f;
                return Pitches;
            }

            // English comment: Normalized progress between 0.0 and 1.0
            const float LinearAlpha =
                FMath::Clamp((RPM - LowerRPM) / RPMRange, 0.0f, 1.0f);

            // English comment: Smooth transition from minimum pitch to 1.0
            // for the lower RPM sound.
            if (LinearAlpha < 0.5f)
            {
                const float Alpha = LinearAlpha * 2.0f;

                Pitches[Index] = FMath::Lerp(
                    MinPitchMultiplier,
                    1.0f,
                    Alpha
                );
            }
            else
            {
                Pitches[Index] = 1.0f;
            }

            // English comment: Smooth transition from 1.0 to maximum pitch
            // for the upper RPM sound.
            if (LinearAlpha > 0.5f)
            {
                const float Alpha = (LinearAlpha - 0.5f) * 2.0f;

                Pitches[Index + 1] = FMath::Lerp(
                    1.0f,
                    MaxPitchMultiplier,
                    Alpha
                );
            }
            else
            {
                Pitches[Index + 1] = 1.0f;
            }

            return Pitches;
        }
    }

    // English comment: Fallback safety net
    Pitches[0] = 1.0f;
    return Pitches;
}