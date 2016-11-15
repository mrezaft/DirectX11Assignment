#pragma once

#include <DirectXMath.h>
#include <GUObject.h>

class LookAtCamera : public GUObject {

private:


	DirectX::XMVECTOR pos;
	DirectX::XMVECTOR up;
	DirectX::XMVECTOR lookAt;

public:

	LookAtCamera() {

		// Build the view matrix.
		pos = DirectX::XMVectorSet(0, 0, -10, 1.0f);
		up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		lookAt = DirectX::XMVectorZero();
	}

	LookAtCamera(DirectX::XMVECTOR init_pos, DirectX::XMVECTOR init_up, DirectX::XMVECTOR init_lookAt) {

		pos = init_pos;
		up = init_up;
		lookAt = init_lookAt;
	}


	//
	// Accessor methods
	//

	void setLookAt(DirectX::XMVECTOR init_lookAt) {

		lookAt = init_lookAt;
	}
	void setPos(DirectX::XMVECTOR init_pos) {

		pos = init_pos;
	}

	DirectX::XMVECTOR getLookAt(DirectX::XMVECTOR init_lookAt) {

		return lookAt;
	}
	DirectX::XMVECTOR getUp(DirectX::XMVECTOR init_lookAt) {

		return up;
	}


	//
	// Camera transformations
	//
	
	void rotateElevation(float t) {

		pos = DirectX::XMVector4Transform(pos, DirectX::XMMatrixRotationX(t));
		

	}

	void rotateOnYAxis(float t) {

		pos = DirectX::XMVector4Transform(pos, DirectX::XMMatrixRotationY(t));
	}

	void zoomCamera(float zoomFactor) {

		pos = DirectX::XMVector4Transform(pos, DirectX::XMMatrixScaling(zoomFactor, zoomFactor, zoomFactor));
	}

	DirectX::XMMATRIX dxViewTransform() {


		return 		DirectX::XMMatrixLookAtLH(pos, lookAt, up);
	}

	DirectX::XMVECTOR getCameraPos() {


		return pos;
	}

};

