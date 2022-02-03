/*Taken reference from practical 5 and 6.The link of the shader is below:
https://www.shadertoy.com/view/tsXXzr
*/

// sphere_trace 
TraceResult sphere_trace(in Ray ray, in SphereTraceDesc params)
{
    TraceResult ret = TraceResult(ray.Tmin, 0,1);
    Value dis ;
    
    int i = 0; do
    {
      dis = sdf(ray.P+ret.T*ray.V);
        ret.mid=dis.matid ;
        ret.T+=dis.d;
        ++i;
    } while (
		ret.T < ray.Tmax &&       			// Stay within bound box
		dis.d	  > params.epsilon * ret.T &&	// Stop if cone is close to surface
		i     < params.maxiters	        	// Stop if too many iterations
	);
    
    ret.flags =  int(ret.T >= ray.Tmax)
              | (int(dis.d <= params.epsilon* ret.T)  << 1)
              | (int(i >= params.maxiters) << 2);
    return ret;
   }
   // normal function
   vec3 normal(const in vec3 p)
   {
    const float eps=0.001;
    vec3 plus = vec3((sdf(p+vec3(eps,0,0))).d,(sdf(p+vec3(0,eps,0))).d,(sdf(p+vec3(0,0,eps))).d);
    vec3 minu = vec3((sdf(p-vec3(eps,0,0))).d,(sdf(p-vec3(0,eps,0))).d,(sdf(p-vec3(0,0,eps))).d);
    return normalize(plus-minu);
    }
   //missColor function
   vec4 missColor(Ray ray)
   {
   return vec4(texture(iChannel2, ray.V).xyz,1.);
   }
   vec4 errorColor(Ray ray, float t)
   {
   return vec4(1.,0.,0.,1.);
    }
   //hitColor function
    vec4 hitColor(Ray ray,float t, int Mid)
  {
    Material mt = matr[Mid] ;
    vec3 p = ray.P + ray.V*t;
    vec3 n = normal(p);
//Two light sources one function of sin and cos another of tan
  
   vec3[] lightsPos = vec3[](vec3(10.*sin(iTime),5.,10.*cos(iTime)),vec3(7.5*cos(iTime),5.5,4.5));
   vec3[] lightsPos1 = vec3[](vec3(10.*tan(iTime),5.,10.*tan(iTime)),vec3(7.5*tan(iTime),5.5,4.5));
    for (int i =0;i<2;i++)
    {
    vec3 v = -ray.V;
    vec3 l = normalize(lightsPos[i]-p);
    vec3 l1 = normalize(lightsPos1[i]-p);
    
    //diffuse
    float costheta = max(dot(n,l),0.);
    float costheta1 = max(dot(n,l1),0.);
    vec3 k_d = mt.color;
    
    //specular
    vec3 toLight = normalize(lightsPos[i] - p);
    vec3 toLight1 = normalize(lightsPos1[i] - p);
    vec3 toEye = -ray.V;
    vec3 k_s = vec3(mt.metalness);
    
    vec3 h = normalize(toLight+toEye);
    vec3 h1 = normalize(toLight1+toEye);
    float si = pow(clamp(dot(h,n),0.,1.),100.);
    float si1 = pow(clamp(dot(h1,n),0.,1.),100.);
    
    //sum
    vec3 col = (k_d + si*k_s)*costheta;
    vec3 col1 = (k_d + si*k_s)*costheta1;
    
    return vec4(col+col1,1.);
    
    }
   
}

// ---- CAMERA and EVENTs ----
// Common key codes (WASD instead of arrows)

const int KeyLeft  = 65;
const int KeyRight = 68;
const int KeyUp    = 87;
const int KeyDown  = 83;

#define isKeyHeld(k)  (texelFetch(iChannel1, ivec2(k,0), 0).x > 0.)

const vec3 EyeStartPosition = vec3(-4.5,0.1,1.5);

Ray Camera(vec2 fragCoord, out vec3 eye, out vec2 data2)
{
    /*
        We will use the first 2 pixels of the buffer to store the information we need.
        Every pixel contains 4 channels (floats), for RGBA. We can exploit this in the following way:
            pixel0 = (empty, cameraX, cameraY, cameraZ)
            pixel1 = (empty, empty, U, V)
        where 
            cameraX, cameraY and cameraZ describe the position of the camera respectively
            U,V give the current rotation of the camera in spherical coordinates
	*/
    
    // Ray generation
    eye = texelFetch(iChannel0, ivec2(0,0), 0).yzw+EyeStartPosition;	 // camera position 
    data2 = texelFetch(iChannel0, ivec2(1,0), 0).zw;	// spherical coordinates
    vec2 uv	= abs(data2);
    
   	if(iMouse.z>0. || data2.x >= 0.)	//mouse held or was held last frame
        uv += (abs(iMouse.zw)-abs(iMouse.xy))*0.01;
    
    vec3 w = vec3(cos(uv.x)*cos(-uv.y),
                  			sin(-uv.y),
                  sin(uv.x)*cos(-uv.y));
    vec3 u = normalize(cross(vec3(0,1,0),w));
	vec3 v = cross(w,u);
    
    vec2 px = (fragCoord/iResolution.xy*2.-1.)*1.*normalize(iResolution.xy);
    
    // Keyboard and mouse handling:
	float speed = 0.2;
    if (isKeyHeld(KeyLeft )) eye -= u*speed;
    if (isKeyHeld(KeyRight)) eye += u*speed;
    if (isKeyHeld(KeyUp   )) eye += w*speed;
    if (isKeyHeld(KeyDown )) eye -= w*speed;
    
    if(iMouse.z>=0.)		//mouse held
        data2 = abs(data2.xy);
	else if(data2.x >= 0.)	//mouse released
        data2 = -mod(uv,2.*pi);
    
    // Ray generation
    return Ray(eye,							//V
               0.5,							//minT
               normalize(w+px.x*u+px.y*v),	//P
               500.);						//maxT
}


void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    time = iTime ;
 // Generate ray from pixel
    vec3 eye; vec2 data; // this will be saved into first two pixels
    Ray ray = Camera(fragCoord, eye, data);
    
    // Set epsilon and maximum iteration
    
    SphereTraceDesc params = SphereTraceDesc(0.006, 1000);
    
    // Raytrace
    TraceResult result = sphere_trace(ray, params);
    
    
    if(bool(result.flags & 1))    fragColor = missColor(ray);    
    else if(bool(result.flags&2)) fragColor = hitColor(ray, result.T,result.mid);
    else 	        			  fragColor = errorColor(ray, result.T);


    vec4 prev = texelFetch(iChannel0, ivec2(fragCoord), 0);
    fragColor = 0.15*prev + 0.4*fragColor;
    fragColor.w = result.T;

    // First two pixels are reserved
    if(fragCoord.x == 0.5 && fragCoord.y == 0.5) // pixel (0,0)
        fragColor.yzw = eye-EyeStartPosition;
    if(fragCoord.x == 1.5 && fragCoord.y == 0.5) //pixel (1,0)
        fragColor.zw = data;

}