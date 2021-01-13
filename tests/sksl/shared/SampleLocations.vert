layout(location=1) noperspective out float2 vcoord_Stage0;
void main()
{
	int x = sk_InstanceID % 200;
	int y = sk_InstanceID / 200;
	int ileft = (sk_InstanceID*929) % 17;
	int iright = ileft + 1 + ((sk_InstanceID*1637) % (17 - ileft));
	int itop = (sk_InstanceID*313) % 17;
	int ibot = itop + 1 + ((sk_InstanceID*1901) % (17 - itop));
	float outset = 1/32.0;
	outset = (0 == (x + y) % 2) ? -outset : +outset;
	float l = float(ileft)/16.0 - outset;
	float r = float(iright)/16.0 + outset;
	float t = float(itop)/16.0 - outset;
	float b = float(ibot)/16.0 + outset;
	float2 vertexpos;
	vertexpos.x = float(x) + ((0 == (sk_VertexID % 2)) ? l : r);
	vertexpos.y = float(y) + ((0 == (sk_VertexID / 2)) ? t : b);
	vcoord_Stage0.x = (0 == (sk_VertexID % 2)) ? -1 : +1;
	vcoord_Stage0.y = (0 == (sk_VertexID / 2)) ? -1 : +1;
	sk_Position = float4(vertexpos.x , vertexpos.y, 0, 1);
}
