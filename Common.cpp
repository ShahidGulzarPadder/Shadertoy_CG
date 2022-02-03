float time ;
const float pi = 3.1415926535897932384626433832795;
struct Ray
{
	vec3 P;
	float Tmin;
	vec3 V;
	float Tmax;
};

struct TraceResult
{
    float T;		// Distance taken on ray
    int flags;
    int mid ;
};                  

// struct SpheretraceDesc declaration
struct SphereTraceDesc
{
    float epsilon;  //Stopping distance to surface
    int maxiters;   //Maximum iteration count
};

// value Material declaration
struct Material {
  vec3 color      ;
  float roughness ;
  vec3 emission ;
  float metalness ;
};

/*
   Material m = matr[matid] ;
   return Ray (m.color,m.roughness,m.emession,m.metalness) ;
*/
const Material[10] matr = Material[10](Material(vec3(1.1,1.6,2.1), 0.5, vec3(0.1,0.1,0.1), 0.03),
                                 Material(vec3(2.1,1.3,0.6), 1.0, vec3(0.2,0.1,0.2), 0.3),
                                 Material(vec3(0.7,2.7,1.5), 1.5, vec3(0.2,0.3,0.3), 0.7),
                                 Material(vec3(1.,0.0,1.0), 2.0, vec3(0.4,0.3,0.8), 0.9),
                                 Material(vec3(2.9,0.5,1.7), 2.5, vec3(0.4,0.3,1.), 0.7),
                                 Material(vec3(2.5,1.5,1.7), 3.0, vec3(0.4,0.3,1.), 0.05),
                                 Material(vec3(3.1,0.5,1.0), 3.5, vec3(0.4,0.3,1.), 0.03),
                                 Material(vec3(2.3,1.2,1.2), 4.0, vec3(0.3,0.3,1.), 0.6),
                                 Material(vec3(0.8,0.5,0.7), 6.0, vec3(0.35,0.3,1.), 0.35),
                                 Material(vec3(1.9,0.5,0.2), 7.0, vec3(0.25,0.3,1.), 0.65));

// value Value declaration
struct Value
{
   float d ;
   int matid;
};
/*PRIMITIVES IDEA FROM:

https://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm

*/

//PLANE PRIMITIVE
Value sdPlane( vec3 p, vec3 n, float h , int matid )
{
  // n must be normalized
  return Value ((dot(p,n) + h),matid);
}

//BOX PRIMITIVE
Value sdBox( vec3 p, vec3 b , int matid)
{
  vec3 q = abs(p) - b;
  return  Value ((length(max(q,0.0)) + min(max(q.x,max(q.y,q.z)),0.0)),matid);
} 

//LINK PRIMITIVE
Value sdLink( vec3 p, float le, float r1, float r2 ,int matid )
{
  vec3 q = vec3( p.x, max(abs(p.y)-le,0.0), p.z );
  return Value ((length(vec2(length(q.xy)-r1,q.z)) - r2),matid);
}

//SPHERE PRIMITIVE
Value sdSphere( vec3 p, float r, int matid)
{
  return Value ((length(p)-r),matid);
}

//TORUS PRIMITIVE
Value sdTorus( vec3 p, vec2 t , int matid)
{ 
  vec2 q = vec2(length(p.xz)-t.x,p.y);
  return Value ((length(q)-t.y),matid);
}
 

//VERTICAL CAPSULE PRIMITIVE
Value sdVerticalCapsule( vec3 p, float h, float r, int matid )
{
  p.y -= clamp( p.y, 0.0, h );
  return Value (length( p ) - r , matid);
}

//OCTAHEDRON PRIMITIVE
Value sdOctahedron( vec3 p, float s , int matid)
{
  p = abs(p);
  return Value (((p.x+p.y+p.z-s)*0.57735027),matid);
}
//CAPSULE PRIMITIVE
Value sdCapsule( vec3 p, vec3 a, vec3 b, float r, int matid)
{
	vec3 pa = p-a, ba = b-a;
	float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
	return Value(length( pa - ba*h ) - r,matid);
}

//IT IS UNION OPERATION
Value unionn(Value A, Value B){
    if (A.d < B.d)
        return A;
    else
        return B;
}
//IT IS INTERSECTION OPERATION
Value intersect(Value A, Value B){
    if (A.d > B.d)
        return A;
    else
        return B;
}
//IT IS DIFFERENCE OPERATION
Value difference(Value a, Value b){
    a.d = -a.d;
    if (a.d > b.d)
        return a;
    else
        return b;
}
//IT IS INVERSION OPERATION
Value invert(Value A){
    A.d = -A.d;
    return A;
}


// DISTANCE FUNCTION

Value sdf(in vec3 p)
{
    float an = tan(time);
    float an1 = sin(time);
    p-=vec3(0,0,1);
    //INITIALIZE THE PLANE, HERE FOR EASE I AM TAKING IT AS GROUND
    Value Ground = sdPlane( p, vec3(0.0,0.05,0.0) , 2.6 , 1);
    
    
    //initialize ELTE Words by boxes
   Value Ob2 = sdBox( p+vec3(0.1,0.1,-6), vec3(0.5*an,3.,0.5) , 2) ;
   Value Ob3 = sdBox( p+vec3(0.1,0.5,-4.5), vec3(0.5*an,0.5,1.) , 3) ;
   Value Ob4 = sdBox( p+vec3(0.1,-2.4,-4.5), vec3(0.5*an,0.5,1.) , 3) ;
   Value Ob5 = sdBox( p+vec3(0.1,-0.9,-5.), vec3(0.5*an,0.5,0.5) , 7) ;
   Value Ob6 = sdBox( p+vec3(0.1,0.1,-2.5), vec3(0.5*an,3.,0.5) , 8) ;
   Value Ob7 = sdBox( p+vec3(0.1,0.5,-1.), vec3(0.5*an,0.5,1.) , 9) ;
   Value Ob8 = sdBox( p+vec3(0.1,-2.4,1.), vec3(0.5*an,0.5,1.5) , 1) ;
   Value Ob9 = sdBox( p+vec3(0.1,0.1,1.), vec3(0.5*an,3.,0.5) , 2) ;
   Value Ob10 = sdBox( p+vec3(0.1,0.1,3.5), vec3(0.5*an,3.,0.5) , 3) ;
   Value Ob11 = sdBox( p+vec3(0.1,0.5,5.0), vec3(0.5*an,0.5,1.) , 2) ;
   Value Ob12 = sdBox( p+vec3(0.1,-2.4,5.0), vec3(0.5*an,0.5,1.) , 6) ;
   Value Ob13 = sdBox( p+vec3(0.1,-0.9,4.6), vec3(0.5*an,0.5,0.6) , 4) ;
   Value Ob14 = sdSphere( p+vec3(0.1,-1.6,-12.0), 1., 5) ;
   Value Ob15 = sdSphere( p+vec3((-1.)*an1,-2.,-12.0), 1., 3) ;
   Value Ob16 = sdSphere( p+vec3(0.1,-1.6,10.0), 1., 5) ;
   Value Ob17 = sdSphere( p+vec3((-1.)*an1,-2.,10.), 1., 3) ;
   Value Ob18 = sdOctahedron( p+vec3(-1.,-6.,(-12.)*an1), 1.5 , 2);
   Value Ob19 = sdTorus( p+vec3(-5.,-6.,0.), vec2(1.5,0.5) , 8) ;
   Value Ob20 = sdVerticalCapsule( p+vec3(-5.,(-6.0)*an,0), 2.,  0.5, 4 );
   Value Ob21 = sdLink( p+vec3(-1.,-6.,(-12.)*an1), 0.1, 0.8, 0.42, 3 );
   Value Ob22 = sdBox( p+vec3(-1.,-0,14), vec3(1.5,5.0,0.5*an1) , 9) ;
   Value Ob23 = sdBox( p+vec3(-1.,-0,12), vec3(1.0,5.0,1.) , 9) ;
   Value Ob24 = sdBox( p+vec3(-1.,-0,-16), vec3(1.5,5.0,0.5*an1) , 9) ;
   Value Ob25 = sdBox( p+vec3(-1.,-0,-18), vec3(1.0,5.0,1.) , 9) ;
   
  ////////////////////////////////////////////////////////////////////////////////////////////
  
  
  //CSG OPERATIONS
  
    Value csg1 = unionn(Ob14,Ob15)  ;
    Value csg2 = unionn(Ob16,Ob17)  ;
    Value csg3 = unionn(Ob8,Ob9)  ;
    Value csg4 = invert(Ob9)  ;
    Value csg5 = intersect(Ob18,Ob21)  ;
    Value csg6 = difference(Ob23,Ob22)  ;
    Value csg7 = difference(Ob25,Ob24)  ;
    
    
    
    //////////////////////////////////////////////////////////////////////////////////
  
    Ground = unionn(Ground,Ob2) ;
    Ground = unionn(Ground,Ob3) ;
    Ground = unionn(Ground,csg2) ;
    Ground = unionn(Ground,Ob4) ;
    Ground = unionn(Ground,Ob5) ;
    Ground = unionn(Ground,Ob6) ;
    Ground = unionn(Ground,Ob7) ;
    Ground = unionn(Ground,csg3) ;
    Ground = unionn(Ground,Ob10) ;
    Ground = unionn(Ground,Ob11) ;
    Ground = unionn(Ground,Ob12) ;
    Ground = unionn(Ground,Ob13) ; 
    Ground = unionn(Ground,csg1) ;
    Ground = unionn(Ground,Ob19) ;
    Ground = unionn(Ground,Ob20) ;
    Ground = unionn(Ground,csg5) ;
    Ground = unionn(Ground,csg6) ;
    Ground = unionn(Ground,csg7) ;
    ////////////////////////////////////////////////////////////////////////////////////
  return  Value(min(p.y+1., Ground.d), Ground.matid) ;
}