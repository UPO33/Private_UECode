bool USSFunctionLibrary::GetMaterialAndUVCoordinatesFromHit(FHitResult HitResult, FMaterialHitResult& OutMaterialHitResult)
{
	OutMaterialHitResult.Material = NULL;
	OutMaterialHitResult.UVCoordinates = FVector2D::ZeroVector;

	UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(HitResult.GetComponent());
	if (StaticMeshComponent)
	{
		FStaticMeshRenderData* RenderData = StaticMeshComponent->StaticMesh->RenderData;

		// Get mesh data from the current LOD.
		const FStaticMeshLODResources& LODResourceData = RenderData->LODResources[StaticMeshComponent->PreviousLODLevel];

		FIndexArrayView IndexArray = LODResourceData.IndexBuffer.GetArrayView();
		uint32 HitVertIndex0 = (uint32)IndexArray[HitResult.FaceIndex * 3];
		uint32 HitVertIndex1 = (uint32)IndexArray[HitResult.FaceIndex * 3 + 1];
		uint32 HitVertIndex2 = (uint32)IndexArray[HitResult.FaceIndex * 3 + 2];

		FVector Vert0Pos = LODResourceData.PositionVertexBuffer.VertexPosition(HitVertIndex0);
		FVector Vert1Pos = LODResourceData.PositionVertexBuffer.VertexPosition(HitVertIndex1);
		FVector Vert2Pos = LODResourceData.PositionVertexBuffer.VertexPosition(HitVertIndex2);

		FVector2D Vert0UV = LODResourceData.VertexBuffer.GetVertexUV(HitVertIndex0, 0);
		FVector2D Vert1UV = LODResourceData.VertexBuffer.GetVertexUV(HitVertIndex1, 0);
		FVector2D Vert2UV = LODResourceData.VertexBuffer.GetVertexUV(HitVertIndex2, 0);

		FVector ComponentLocation = StaticMeshComponent->GetComponentLocation();
		FRotator ComponentRotation = StaticMeshComponent->GetComponentRotation();
		FVector ComponentScale = StaticMeshComponent->GetComponentScale();

		// Transform world impact point to local coordinates.
		FVector LocalImpactPoint = ComponentRotation.UnrotateVector(HitResult.ImpactPoint - ComponentLocation) / ComponentScale;

		// Determine the barycentric coordinates
		FVector u = Vert1Pos - Vert0Pos;
		FVector v = Vert2Pos - Vert0Pos;
		FVector w = LocalImpactPoint - Vert0Pos;

		FVector vCrossW = FVector::CrossProduct(v, w);
		FVector vCrossU = FVector::CrossProduct(v, u);

		if (FVector::DotProduct(vCrossW, vCrossU) < 0.0f)
		{
			return false;
		}

		FVector uCrossW = FVector::CrossProduct(u, w);
		FVector uCrossV = FVector::CrossProduct(u, v);

		if (FVector::DotProduct(uCrossW, uCrossV) < 0.0f)
		{
			return false;
		}

		float denom = uCrossV.Size();
		float b1 = vCrossW.Size() / denom;
		float b2 = uCrossW.Size() / denom;
				float b0 = 1.0f - b1 - b2;

		// Determine the hit UV.
		float hitU = b0 * Vert0UV.X + b1 * Vert1UV.X + b2 * Vert2UV.X;
		float hitV = b0 * Vert0UV.Y + b1 * Vert1UV.Y + b2 * Vert2UV.Y;

		OutMaterialHitResult.UVCoordinates = FVector2D(hitU, hitV);

		// Loop through the LOD's sections to find the one that contains the face that was hit.
		for (int32 i = 0; i < LODResourceData.Sections.Num(); i++)
		{
			uint32 MinVertIndex = LODResourceData.Sections[i].MinVertexIndex;
			uint32 MaxVertIndex = LODResourceData.Sections[i].MaxVertexIndex;

			if (FMath::IsWithinInclusive(HitVertIndex0, MinVertIndex, MaxVertIndex) &
				FMath::IsWithinInclusive(HitVertIndex1, MinVertIndex, MaxVertIndex) &
				FMath::IsWithinInclusive(HitVertIndex2, MinVertIndex, MaxVertIndex))
			{
				OutMaterialHitResult.Material = StaticMeshComponent->GetMaterial(LODResourceData.Sections[i].MaterialIndex);
				break;
			}
		}

		if (OutMaterialHitResult.Material == NULL)
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	return true;
}

Hey JGagner, I'm somewhat new to the C++ arena and even more so how it ties into UE4. Please pardon my ignorance, did you create a custom class for this function from the editor, or perhaps you embedded this code in your project/engine files? I am attempting to find the UV coordinates of a mesh rather than the triangles, 1 UV map for a sphere for example. (it seems that this is providing that solution tough I'm not entirely certain). Please reply. So far I have used a trace method which after a few hours of troubleshooting has produced UVs of triangles (thanks Rama), but not of the overall mesh.

Thanks 

Quick reply to this message Reply   Reply With Quote Reply With Quote   Multi-Quote This Message      
05-13-2016, 02:15 PM #6
0
Tom Looman
Tom Looman is offline
Champion

Join Date
Mar 2014
Posts
628	

Quote Originally Posted by JGagner View Post
Thanks for the replies. Custom code was indeed the way to go. Turns out it really wasn't all that hard to get the information needed to calculate the UV's for the hit. The material was equally as easy.

Here's the code I ended up with in-case anyone is in a similar situation. I'm not entirely sure if it's the best way but it worked for me.

Code:

struct FMaterialHitResult
{
	GENERATED_USTRUCT_BODY()
public:
	UMaterialInterface* Material;

	FVector2D UVCoordinates;

	FMaterialHitResult()
	{
		Material = NULL;
		UVCoordinates = FVector2D::ZeroVector;
	}

};

Code:

bool USSFunctionLibrary::GetMaterialAndUVCoordinatesFromHit(FHitResult HitResult, FMaterialHitResult& OutMaterialHitResult)
{
	OutMaterialHitResult.Material = NULL;
	OutMaterialHitResult.UVCoordinates = FVector2D::ZeroVector;

	UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(HitResult.GetComponent());
	if (StaticMeshComponent)
	{
		FStaticMeshRenderData* RenderData = StaticMeshComponent->StaticMesh->RenderData;

		// Get mesh data from the current LOD.
		const FStaticMeshLODResources& LODResourceData = RenderData->LODResources[StaticMeshComponent->PreviousLODLevel];

		FIndexArrayView IndexArray = LODResourceData.IndexBuffer.GetArrayView();
		uint32 HitVertIndex0 = (uint32)IndexArray[HitResult.FaceIndex * 3];
		uint32 HitVertIndex1 = (uint32)IndexArray[HitResult.FaceIndex * 3 + 1];
		uint32 HitVertIndex2 = (uint32)IndexArray[HitResult.FaceIndex * 3 + 2];

		FVector Vert0Pos = LODResourceData.PositionVertexBuffer.VertexPosition(HitVertIndex0);
		FVector Vert1Pos = LODResourceData.PositionVertexBuffer.VertexPosition(HitVertIndex1);
		FVector Vert2Pos = LODResourceData.PositionVertexBuffer.VertexPosition(HitVertIndex2);

		FVector2D Vert0UV = LODResourceData.VertexBuffer.GetVertexUV(HitVertIndex0, 0);
		FVector2D Vert1UV = LODResourceData.VertexBuffer.GetVertexUV(HitVertIndex1, 0);
		FVector2D Vert2UV = LODResourceData.VertexBuffer.GetVertexUV(HitVertIndex2, 0);

		FVector ComponentLocation = StaticMeshComponent->GetComponentLocation();
		FRotator ComponentRotation = StaticMeshComponent->GetComponentRotation();
		FVector ComponentScale = StaticMeshComponent->GetComponentScale();

		// Transform world impact point to local coordinates.
		FVector LocalImpactPoint = ComponentRotation.UnrotateVector(HitResult.ImpactPoint - ComponentLocation) / ComponentScale;

		// Determine the barycentric coordinates
		FVector u = Vert1Pos - Vert0Pos;
		FVector v = Vert2Pos - Vert0Pos;
		FVector w = LocalImpactPoint - Vert0Pos;

		FVector vCrossW = FVector::CrossProduct(v, w);
		FVector vCrossU = FVector::CrossProduct(v, u);

		if (FVector::DotProduct(vCrossW, vCrossU) < 0.0f)
		{
			return false;
		}

		FVector uCrossW = FVector::CrossProduct(u, w);
		FVector uCrossV = FVector::CrossProduct(u, v);

		if (FVector::DotProduct(uCrossW, uCrossV) < 0.0f)
		{
			return false;
		}

		float denom = uCrossV.Size();
		float b1 = vCrossW.Size() / denom;
		float b2 = uCrossW.Size() / denom;
				float b0 = 1.0f - b1 - b2;

		// Determine the hit UV.
		float hitU = b0 * Vert0UV.X + b1 * Vert1UV.X + b2 * Vert2UV.X;
		float hitV = b0 * Vert0UV.Y + b1 * Vert1UV.Y + b2 * Vert2UV.Y;

		OutMaterialHitResult.UVCoordinates = FVector2D(hitU, hitV);

		// Loop through the LOD's sections to find the one that contains the face that was hit.
		for (int32 i = 0; i < LODResourceData.Sections.Num(); i++)
		{
			uint32 MinVertIndex = LODResourceData.Sections[i].MinVertexIndex;
			uint32 MaxVertIndex = LODResourceData.Sections[i].MaxVertexIndex;

			if (FMath::IsWithinInclusive(HitVertIndex0, MinVertIndex, MaxVertIndex) &
				FMath::IsWithinInclusive(HitVertIndex1, MinVertIndex, MaxVertIndex) &
				FMath::IsWithinInclusive(HitVertIndex2, MinVertIndex, MaxVertIndex))
			{
				OutMaterialHitResult.Material = StaticMeshComponent->GetMaterial(LODResourceData.Sections[i].MaterialIndex);
				break;
			}
		}

		if (OutMaterialHitResult.Material == NULL)
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	return true;
}

Mind sharing the includes you used?

Did you look at Rama's solution for this issue? https://wiki.unrealengine.com/Line_T...Info_From_Hits 

Twitter | tomlooman.com | Engine Content Creator @ Epic Games - C++ Survival Game Series | Programmer for Switch (Multiplayer FPS) | Check out my Unreal Engine 4 Resource Collection

Quick reply to this message Reply   Reply With Quote Reply With Quote   Multi-Quote This Message      
05-14-2016, 12:33 AM #7
0
JGagner
JGagner is offline
Supporter

Join Date
Mar 2015
Posts
6	

Hey Tom, I have seen the code Rama is using. There is a slight issue with it, however, it does not in fact return the UVs as one would typically think of them. What it does return is actually the barycentric coordinates of the triangle that the trace hit. I looked and looked but it seemed that PhysX does not provide a way to get the UV channel information as that is stored by the engine elsewhere. As for the includes I required to make the code work it was simply just
Code:

#include "Runtime/Engine/Public/StaticMeshResources.h"

Sadly the solution I came up with only works on static meshes for the time being... Maybe one day I will find a better solution for this but it works for my purposes as of now. 

Quick reply to this message Reply   Reply With Quote Reply With Quote   Multi-Quote This Message      
05-30-2016, 07:26 PM #8
0
Tom Looman
Tom Looman is offline
Champion

Join Date
Mar 2014
Posts
628	

Quote Originally Posted by JGagner View Post
Hey Tom, I have seen the code Rama is using. There is a slight issue with it, however, it does not in fact return the UVs as one would typically think of them. What it does return is actually the barycentric coordinates of the triangle that the trace hit. I looked and looked but it seemed that PhysX does not provide a way to get the UV channel information as that is stored by the engine elsewhere. As for the includes I required to make the code work it was simply just
Code:

#include "Runtime/Engine/Public/StaticMeshResources.h"

Sadly the solution I came up with only works on static meshes for the time being... Maybe one day I will find a better solution for this but it works for my purposes as of now.
Yep I found that out while trying to implement his code.
I ended up working around it for a demo, but that workaround only works on simple planes.

Twitter | tomlooman.com | Engine Content Creator @ Epic Games - C++ Survival Game Series | Programmer for Switch (Multiplayer FPS) | Check out my Unreal Engine 4 Resource Collection

Quick reply to this message Reply   Reply With Quote Reply With Quote   Multi-Quote This Message      
06-23-2016, 12:59 PM #9
0
Crow87
Crow87 is offline
Veteran

Join Date
Mar 2014
Posts
385	

Hey,

Sorry to necro an ancient thread, but I've just recently started exploring this code again. I've implemented your code as an actor component on my player pawn, and everything seems to work - except the UV coordinates. The component correctly identifies the material on the mesh, but the UV coordinates only ever return (0,0). I'm currently trying this in 4.12.3, so if something changed between your original version, and the current engine build, that'd be handy to know.

Any ideas what's going on?

UITrace.h
Code:

#pragma once

#include "Components/ActorComponent.h"
#include "Runtime/Engine/Public/StaticMeshResources.h"
#include "UITrace.generated.h"


USTRUCT(BlueprintType)
struct FMaterialHitResult
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	UMaterialInterface* Material;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
	FVector2D UVCoordinates;

	FMaterialHitResult()
	{
		Material = NULL;
		UVCoordinates = FVector2D::ZeroVector;
	}

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class INTERACTABLES_API UUITrace : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UUITrace();

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

	UFUNCTION(BlueprintCallable, Category = "Interaction")
	bool GetMaterialAndUVCoordinatesFromHit(FHitResult HitResult, FMaterialHitResult& OutMaterialHitResult);
	
};

UITrace.cpp
Code:

#include "Interactables.h"
#include "Runtime/Engine/Public/StaticMeshResources.h"
#include "UITrace.h"


// Sets default values for this component's properties
UUITrace::UUITrace()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UUITrace::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UUITrace::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

	// ...
}

bool UUITrace::GetMaterialAndUVCoordinatesFromHit(FHitResult HitResult, FMaterialHitResult& OutMaterialHitResult)
{
	OutMaterialHitResult.Material = NULL;
	OutMaterialHitResult.UVCoordinates = FVector2D::ZeroVector;

	UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(HitResult.GetComponent());
	if (StaticMeshComponent)
	{
		FStaticMeshRenderData* RenderData = StaticMeshComponent->StaticMesh->RenderData;

		// Get mesh data from the current LOD.
		const FStaticMeshLODResources& LODResourceData = RenderData->LODResources[StaticMeshComponent->PreviousLODLevel];

		FIndexArrayView IndexArray = LODResourceData.IndexBuffer.GetArrayView();
		uint32 HitVertIndex0 = (uint32)IndexArray[HitResult.FaceIndex * 3];
		uint32 HitVertIndex1 = (uint32)IndexArray[HitResult.FaceIndex * 3 + 1];
		uint32 HitVertIndex2 = (uint32)IndexArray[HitResult.FaceIndex * 3 + 2];

		FVector Vert0Pos = LODResourceData.PositionVertexBuffer.VertexPosition(HitVertIndex0);
		FVector Vert1Pos = LODResourceData.PositionVertexBuffer.VertexPosition(HitVertIndex1);
		FVector Vert2Pos = LODResourceData.PositionVertexBuffer.VertexPosition(HitVertIndex2);

		FVector2D Vert0UV = LODResourceData.VertexBuffer.GetVertexUV(HitVertIndex0, 0);
		FVector2D Vert1UV = LODResourceData.VertexBuffer.GetVertexUV(HitVertIndex1, 0);
		FVector2D Vert2UV = LODResourceData.VertexBuffer.GetVertexUV(HitVertIndex2, 0);

		FVector ComponentLocation = StaticMeshComponent->GetComponentLocation();
		FRotator ComponentRotation = StaticMeshComponent->GetComponentRotation();
		FVector ComponentScale = StaticMeshComponent->GetComponentScale();

		// Transform world impact point to local coordinates.
		FVector LocalImpactPoint = ComponentRotation.UnrotateVector(HitResult.ImpactPoint - ComponentLocation) / ComponentScale;

		// Determine the barycentric coordinates
		FVector u = Vert1Pos - Vert0Pos;
		FVector v = Vert2Pos - Vert0Pos;
		FVector w = LocalImpactPoint - Vert0Pos;

		FVector vCrossW = FVector::CrossProduct(v, w);
		FVector vCrossU = FVector::CrossProduct(v, u);

		if (FVector::DotProduct(vCrossW, vCrossU) < 0.0f)
		{
			return false;
		}

		FVector uCrossW = FVector::CrossProduct(u, w);
		FVector uCrossV = FVector::CrossProduct(u, v);

		if (FVector::DotProduct(uCrossW, uCrossV) < 0.0f)
		{
			return false;
		}

		float denom = uCrossV.Size();
		float b1 = vCrossW.Size() / denom;
		float b2 = uCrossW.Size() / denom;
		float b0 = 1.0f - b1 - b2;

		// Determine the hit UV.
		float hitU = (b0 * Vert0UV.X) + (b1 * Vert1UV.X) + (b2 * Vert2UV.X);
		float hitV = (b0 * Vert0UV.Y) + (b1 * Vert1UV.Y) + (b2 * Vert2UV.Y);


		OutMaterialHitResult.UVCoordinates = FVector2D(hitU, hitV);

		// Loop through the LOD's sections to find the one that contains the face that was hit.
		for (int32 i = 0; i < LODResourceData.Sections.Num(); i++)
		{
			uint32 MinVertIndex = LODResourceData.Sections[i].MinVertexIndex;
			uint32 MaxVertIndex = LODResourceData.Sections[i].MaxVertexIndex;

			if (FMath::IsWithinInclusive(HitVertIndex0, MinVertIndex, MaxVertIndex) &
				FMath::IsWithinInclusive(HitVertIndex1, MinVertIndex, MaxVertIndex) &
				FMath::IsWithinInclusive(HitVertIndex2, MinVertIndex, MaxVertIndex))
			{
				OutMaterialHitResult.Material = StaticMeshComponent->GetMaterial(LODResourceData.Sections[i].MaterialIndex);
				break;
			}
		}

		if (OutMaterialHitResult.Material == NULL)
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	return true;
}

Last edited by Crow87; 06-23-2016 at 01:02 PM. 

Latest Company Showreel
http://vimeo.com/116345137

Quick reply to this message Reply   Reply With Quote Reply With Quote   Multi-Quote This Message      

