// Copyright 1998-2017 Epic Games, Inc. All Rights Reserved.

/*=============================================================================
	WindowsD3D11ConstantBuffer.cpp: D3D Constant Buffer functions
=============================================================================*/

#include "WindowsD3D11ConstantBuffer.h"
#include "D3D11RHIPrivate.h"

void FWinD3D11ConstantBuffer::InitDynamicRHI()
{
	TRefCountPtr<ID3D11Buffer> CBuffer = NULL;
	D3D11_BUFFER_DESC BufferDesc;
	ZeroMemory( &BufferDesc, sizeof( D3D11_BUFFER_DESC ) );
	// Verify constant buffer ByteWidth requirements
	check(MaxSize <= D3D11_REQ_CONSTANT_BUFFER_ELEMENT_COUNT && (MaxSize % 16) == 0);
	BufferDesc.ByteWidth = MaxSize;

	BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	BufferDesc.CPUAccessFlags = 0;
	BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	BufferDesc.MiscFlags = 0;

	Buffers = new TRefCountPtr<ID3D11Buffer>[NumSubBuffers];
	CurrentSubBuffer = 0;
	for(uint32 s = 0;s < NumSubBuffers;s++)
	{
		VERIFYD3D11RESULT_EX(D3DRHI->GetDevice()->CreateBuffer(&BufferDesc, NULL, Buffers[s].GetInitReference()), D3DRHI->GetDevice());
		UpdateBufferStats(Buffers[s],true);
		BufferDesc.ByteWidth = Align(BufferDesc.ByteWidth / 2, 16);
	}

	FD3D11ConstantBuffer::InitDynamicRHI();
}

void FWinD3D11ConstantBuffer::ReleaseDynamicRHI()
{
	FD3D11ConstantBuffer::ReleaseDynamicRHI();

	if(Buffers)
	{
		for(uint32 s = 0;s < NumSubBuffers;s++)
		{
			UpdateBufferStats(Buffers[s],false);
		}

		delete [] Buffers;
		Buffers = NULL;
	}
}

bool FWinD3D11ConstantBuffer::CommitConstantsToDevice( bool bDiscardSharedConstants )
{
	// New circular buffer system for faster constant uploads.  Avoids CopyResource and speeds things up considerably
	if (CurrentUpdateSize == 0)
	{
		return false;
	}

	SCOPE_CYCLE_COUNTER(STAT_D3D11GlobalConstantBufferUpdateTime);

	if ( bDiscardSharedConstants )
	{
		// If we're discarding shared constants, just use constants that have been updated since the last Commit.
		TotalUpdateSize = CurrentUpdateSize;
	}
	else
	{
		// If we're re-using shared constants, use all constants.
		TotalUpdateSize = FMath::Max( CurrentUpdateSize, TotalUpdateSize );
	}

	// This basically keeps track dynamically how much data has been updated every frame
	// and then divides up a "max" constant buffer size by halves down until it finds a large sections that more tightly covers
	// the amount updated, assuming that all data in a constant buffer is updated each draw call and contiguous.
	// This only used for the IndexSlot==0 constant buffer on the vertex shader.
	// Which will have an indeterminate number of constant values that are generated by material shaders.
	CurrentSubBuffer = 1;
	uint32 BufferSize = MaxSize / 2;
	while ( BufferSize >= TotalUpdateSize && CurrentSubBuffer < NumSubBuffers )
	{
		CurrentSubBuffer++;
		BufferSize /= 2;
	}
	CurrentSubBuffer--;
	BufferSize *= 2;

	ID3D11Buffer* Buffer = Buffers[CurrentSubBuffer];
	check(((uint64)(ShadowData) & 0xf) == 0);
	D3DRHI->GetDeviceContext()->UpdateSubresource(Buffer,0,NULL,(void*)ShadowData,BufferSize,BufferSize);

	CurrentUpdateSize = 0;

	return true;
}

void FD3D11DynamicRHI::InitConstantBuffers()
{
	// Allocate shader constant buffers.  All shader types can have access to all buffers.
	//  Index==0 is reserved for "custom" params, and the rest are reserved by the system for Common
	//	constants
	VSConstantBuffers.Empty(MAX_CONSTANT_BUFFER_SLOTS);
	PSConstantBuffers.Empty(MAX_CONSTANT_BUFFER_SLOTS);
	HSConstantBuffers.Empty(MAX_CONSTANT_BUFFER_SLOTS);
	DSConstantBuffers.Empty(MAX_CONSTANT_BUFFER_SLOTS);
	GSConstantBuffers.Empty(MAX_CONSTANT_BUFFER_SLOTS);
	CSConstantBuffers.Empty(MAX_CONSTANT_BUFFER_SLOTS);
	for(int32 BufferIndex = 0;BufferIndex < MAX_CONSTANT_BUFFER_SLOTS;BufferIndex++)
	{
		uint32 Size = GConstantBufferSizes[BufferIndex];
		uint32 SubBuffers = 1;
		if(BufferIndex == GLOBAL_CONSTANT_BUFFER_INDEX)
		{
			SubBuffers = 5;
		}

		// Vertex shader can have subbuffers for index==0.  This is from Epic's original design for the auto-fit of size to
		//	reduce the update costs of the buffer.

		// New circular buffer system for faster constant uploads.  Avoids CopyResource and speeds things up considerably
		VSConstantBuffers.Add(new FWinD3D11ConstantBuffer(this, Size, SubBuffers));
		PSConstantBuffers.Add(new FWinD3D11ConstantBuffer(this, Size, SubBuffers));
		HSConstantBuffers.Add(new FWinD3D11ConstantBuffer(this, Size));
		DSConstantBuffers.Add(new FWinD3D11ConstantBuffer(this, Size));
		GSConstantBuffers.Add(new FWinD3D11ConstantBuffer(this, Size));
		CSConstantBuffers.Add(new FWinD3D11ConstantBuffer(this, Size));
	}
}

DEFINE_STAT(STAT_D3D11GlobalConstantBufferUpdateTime);
