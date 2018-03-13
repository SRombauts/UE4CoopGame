// Copyright 2018 Sebastien Rombauts (sebastien.rombauts@gmail.com)

#pragma once

#include "CoreMinimal.h"

// The following should be defined accordingly with the Project Physics Properties
// in DefaultEngine.ini:
// + PhysicalSurfaces = (Type = SurfaceType1, Name = "FleshDefault")
// + PhysicalSurfaces = (Type = SurfaceType2, Name = "FleshVulnerable")
#define SURFACE_FLESH_DEFAULT			EPhysicalSurface::SurfaceType1
#define SURFACE_FLESH_VULNERABLE		EPhysicalSurface::SurfaceType2

#define COLLISION_WEAPON				ECollisionChannel::ECC_GameTraceChannel1