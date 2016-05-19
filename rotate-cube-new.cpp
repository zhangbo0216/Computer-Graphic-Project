
#include "Angel-yjc.h"

typedef Angel::vec4  color4;
typedef Angel::vec4  point4;

std::string filename;
GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile);

GLuint program,program2,programfire;       /* shader program object id */
GLuint floor_buffer,shaded_floor_buffer;  /* vertex buffer object id for floor */
GLuint coordinate_buffer; /* vertex buffer object id for coordinate */
GLuint sphere_buffer,flat_sphere_buffer,smooth_sphere_buffer;     /* vertex buffer object id for sphere */
GLuint shadow_buffer;
GLuint firework_buffer;

// Projection transformation parameters
GLfloat  fovy = 53.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.1, zFar = 40;

GLfloat angle = 0.0; // rotation angle in degrees
vec3 move={0, 0, 0}; // vectpr for moving
vec3 A={-4,1,4},B={-1,1,-4},C={3,1,5},AB=B-A,BC=C-B,CA=A-C,tw=AB;
mat4 Mrotate= mat4( vec4(1,0,0,0),
                   vec4(0,1,0,0),
                   vec4(0,0,1,0),
                   vec4(0,0,0,1)); //matrix to store rotation
int zone=1,sum=0;

vec4 init_eye(7.0, 3.0,-10.0, 1.0); // initial viewer position
vec4 eye = init_eye;               // current viewer position

int animationFlag = 1; // 1: non-animation; 1: non-animation. Toggled by key 'a' or 'A'
bool avaliable = false;
int smoothFlag = 1;
int shadowFlag=1;
int WireFramFlag=0;
int LightingFlag=1;
int spotLightFlag=0;
int fogFlag	= 0;
int textureFlag=1;
int sphereTexFlag=0;
int draw=0;
int verticalFlag=1;
int eyeSpaceFlag=0;
int latticeFlag=0;
int upLatticeFlag=1;
int fireworkFlag=1;
int blendingShadowFlag=1;

int cubeFlag = 1;   // 1: solid cube; 0: wireframe cube. Toggled by key 'c' or 'C'
int floorFlag = 1;  // 1: solid floor; 0: wireframe floor. Toggled by key 'f' or 'F'
int startFlag = 1;  // 1: dont't move; 0: start to roll. Toggled by key 'b' or 'B'
int sphere_numVertices;


/*firework*/
float time_Old = 0.0;
float time_New = 0.0;
float time_Sub = 0.0;
float time_Max = 10000.0;

/*--- Texture ---*/
#define checkerWidth  32
#define checkerHeight 32
GLubyte checkerImage[checkerHeight][checkerWidth][4];

#define	stripeWidth 32
GLubyte stripeImage[4 * stripeWidth];

GLuint checkerTex;								// Checkboard texture
GLuint stripeTex;								// Stripe texture

vec2 texCoord[6] = {
    vec2(0.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0),
    
    vec2(1.0, 1.0),
    vec2(1.0, 0.0),
    vec2(0.0, 0.0),
};



point4 sphere_points[1024*3];
color4 sphere_color[1024*3];
vec3   flat_normals[1024*3];
vec3   smooth_normals[1024*3];

point4 shadow_points[1024*3];
point4 shadow_colors[1024*3];

point4 coordinate_point[9];
color4 coordinate_color[9];


const int floor_NumVertices = 6;
point4 floor_points[floor_NumVertices]; // positions for all vertices
color4 floor_colors[floor_NumVertices]; // colors for all vertices
vec3  floor_normals[floor_NumVertices];
//----------------------------------------------------------------------------

std::string chooseFile()
{
    printf("Enter: 1. sphere.8  2.sphere.128 3.sphere.256 4.sphere.1024\n");
    int choose;
    std::cin >> choose;
    if (choose == 1) return "sphere.8";
    else if (choose == 2) return "sphere.128";
	else if (choose == 3) return "sphere.256"; 
	else return "sphere.1024";
}
//------------------------------------------------------------------------------
void Sphere(std::string filename)
{
    std::ifstream f(filename, std::ios::in);
    int tem;
    f >> sphere_numVertices;
    for (int i=0;i<sphere_numVertices;i++)
    {
        
        f>>tem;
        float a,b,c;
        for (int j=0;j<tem;j++)
        {
            f>>a>>b>>c;
            sphere_points[i*3+j]=point4(a,b,c,1);
            sphere_color[i*3+j] = color4(1.0,0.84,0,1);
            
        }
    }
    sphere_numVertices*=3;
    for (int i = 0; i < sphere_numVertices; i += 3) {
        int a = i;
        int b = i + 1;
        int c = i + 2;
        
        vec4 u = sphere_points[b] - sphere_points[a];
        vec4 v = sphere_points[c] - sphere_points[a];
        vec3 normal = normalize(cross(u, v));
        
        flat_normals[a] = normal; smooth_normals[a] = vec3(sphere_points[a].x, sphere_points[a].y, sphere_points[a].z);
        flat_normals[b] = normal; smooth_normals[b] = vec3(sphere_points[b].x, sphere_points[b].y, sphere_points[b].z);
        flat_normals[c] = normal; smooth_normals[c] = vec3(sphere_points[c].x, sphere_points[c].y, sphere_points[c].z);
    }


}

void setup_texture(void) {
    int i, j, c;
    for (i = 0; i < checkerHeight; i++) {
        for (j = 0; j < checkerWidth; j++) {
            c = (((i & 0x8) == 0) ^ ((j & 0x8) ==0));
            
            /*-- c == 1: white, else brown --*/
            checkerImage[i][j][0] = (GLubyte) ((c==1) ? 255 : 0);
            checkerImage[i][j][1] = (GLubyte) ((c==1) ? 255 : 150);
            checkerImage[i][j][2] = (GLubyte) ((c==1) ? 255 : 0);
            checkerImage[i][j][3] = (GLubyte) 255; } }
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    
    /*--- Create stripe image ---*/
    for (j = 0; j < stripeWidth; j++) {
        stripeImage[4 * j] = (GLubyte)255;
        stripeImage[4 * j + 1] = (GLubyte)((j>4) ? 255 : 0);
        stripeImage[4 * j + 2] = (GLubyte)0;
        stripeImage[4 * j + 3] = (GLubyte)255;
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}



void coordinate()
{
    coordinate_point[0] = point4(0,0,0,1); coordinate_color[0] = color4(1,0,0,1);
    coordinate_point[1] = point4(10,0,0,1); coordinate_color[1] = color4(1,0,0,1);
    coordinate_point[2] = point4(10,0,0,1); coordinate_color[2] = color4(1,0,0,1);
    
    coordinate_point[3] = point4(0,0,0,1);  coordinate_color[3] = color4(1,0,1,1);
    coordinate_point[4] = point4(0,10,0,1); coordinate_color[4] = color4(1,0,1,1);
    coordinate_point[5] = point4(0,10,0,1); coordinate_color[5] = color4(1,0,1,1);
    
    coordinate_point[6] = point4(0,0,0,1);  coordinate_color[6] = color4(0,0,1,1);
    coordinate_point[7] = point4(0,0,10,1); coordinate_color[7] = color4(0,0,1,1);
    coordinate_point[8] = point4(0,0,10,1); coordinate_color[8] = color4(0,0,1,1);
    
}

void shadow()
{
    for (int i = 0;i < sphere_numVertices;i++)
    {
        //int x = sphere_points[i].x, y = sphere_points[i].y, z = sphere_points[i].z;
        //shadow_points[i]=point4(x,y,z,1);
        shadow_colors[i]=color4(0.25, 0.25, 0.25, 0.65);
    }
}

// generate 2 triangles: 6 vertices and 1 color
void floor()
{
    
    
    floor_colors[0] = color4(0,1,0,1); floor_points[0] = point4(5,  0, 8,1);
    floor_colors[1] = color4(0,1,0,1); floor_points[1] = point4(5, 0, -4,1);
    floor_colors[2] = color4(0,1,0,1); floor_points[2] = point4(-5, 0,-4,1);
    
    floor_colors[3] = color4(0,1,0,1); floor_points[3] = point4(5, 0, 8,1);
    floor_colors[4] = color4(0,1,0,1); floor_points[4] = point4(-5, 0, 8,1);
    floor_colors[5] = color4(0,1,0,1); floor_points[5] = point4(-5, 0,-4,1);
    vec4 u = point4( 5.0, 0.0, -4.0, 1.0) - point4( 5.0, 0.0,  8.0, 1.0);
    vec4 v = point4(-5.0, 0.0,  8.0, 1.0) - point4( 5.0, 0.0,  8.0, 1.0);
    vec3 normal = normalize(cross(u, v));
    for (int i=0;i<6;i++)
        floor_normals[i]=normal;
}


const int N = 300;							// number of particles in particle system
point4	  fireworks_points[N];				// position of each particle
vec3	  fireworks_velocities[N];			// velocities of particles
color4	  fireworks_colors[N];				// color of each particle


void firework()
{
    int i;
    for (i = 0; i < N; i++){
        fireworks_points[i] = point4(0.0, 0.1, 0.0, 1.0);
        
        // Assigning random velocity to particles
        fireworks_velocities[i].x = 2.0 * ((rand() % 256) / 256.0 - 0.5);
        fireworks_velocities[i].y = 1.2 * 2.0 * (rand() % 256) / 256.0;
        fireworks_velocities[i].z = 2.0 * ((rand() % 256) / 256.0 - 0.5);
        
        // Assigning random color to particles
        fireworks_colors[i].x = (rand() % 256) / 256.0;
        fireworks_colors[i].y = (rand() % 256) / 256.0;
        fireworks_colors[i].z = (rand() % 256) / 256.0;
        fireworks_colors[i].w = 1.0;
    }
}


//----------------------------------------------------------------------------
// OpenGL initialization
void init()
{
    
    setup_texture();
    
    // Checkerboard Texture
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &checkerTex);					// Generate texture obj
    glActiveTexture(GL_TEXTURE0);					// Set the active texture to be 0
    glBindTexture(GL_TEXTURE_2D, checkerTex);		// Bind texture to texture unit
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, checkerWidth, checkerHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkerImage);
    
    // Stripe Texture
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &stripeTex);					// Generate texture obj
    glActiveTexture(GL_TEXTURE1);					// Set the active texture to be 1
    glBindTexture(GL_TEXTURE_1D, stripeTex);		// Bind texture to texture unit
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, stripeWidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, stripeImage);
    
    
    
    
    floor();
    // Create and initialize a vertex buffer object for floor, to be used in display()
    glGenBuffers(1, &floor_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floor_points) + sizeof(floor_colors),
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_colors),
                    floor_colors);
    
    //buffer for shaded floor
    glGenBuffers(1, &shaded_floor_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, shaded_floor_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4)* floor_NumVertices + sizeof(vec3)* floor_NumVertices+sizeof(texCoord), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4)* floor_NumVertices, floor_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4)* floor_NumVertices, sizeof(vec3)* floor_NumVertices, floor_normals);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points)+sizeof(floor_normals), sizeof(texCoord), texCoord);
    
    
    coordinate();
    // buffer for coordinate
    glGenBuffers(1, &coordinate_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, coordinate_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(coordinate_point) + sizeof(coordinate_color)+sizeof(texCoord),
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(coordinate_point), coordinate_point);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(coordinate_point), sizeof(coordinate_color),
                    coordinate_color);
    
    
    
    
    Sphere(filename);
    //buffer for shpere
    glGenBuffers(1, &sphere_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4)* sphere_numVertices + sizeof(color4)* sphere_numVertices, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4)* sphere_numVertices, sphere_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4)* sphere_numVertices, sizeof(color4)* sphere_numVertices, sphere_color);
    
    //buffer for flat_shpere
    glGenBuffers(1, &flat_sphere_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, flat_sphere_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4)* sphere_numVertices + sizeof(vec3)* sphere_numVertices+sizeof(texCoord), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4)* sphere_numVertices, sphere_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4)* sphere_numVertices, sizeof(vec3)* sphere_numVertices, flat_normals);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4)* sphere_numVertices + sizeof(vec3)* sphere_numVertices, sizeof(texCoord), texCoord);

    
    //buffer for smooth_shpere
    glGenBuffers(1, &smooth_sphere_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, smooth_sphere_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4)* sphere_numVertices + sizeof(vec3)* sphere_numVertices+sizeof(texCoord), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4)* sphere_numVertices, sphere_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4)* sphere_numVertices, sizeof(vec3)* sphere_numVertices, smooth_normals);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4)* sphere_numVertices + sizeof(vec3)* sphere_numVertices, sizeof(texCoord), texCoord);

    
    shadow();
    
    //buffer for shadow
    glGenBuffers(1, &shadow_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, shadow_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_numVertices + sizeof(color4) * sphere_numVertices,
                 NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(point4) * sphere_numVertices, sphere_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(point4) * sphere_numVertices, sizeof(color4) * sphere_numVertices, shadow_colors);
    
    //buffer for firework
    firework();
    glGenBuffers(1, &firework_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, firework_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(fireworks_points)+sizeof(fireworks_colors)+sizeof(fireworks_velocities), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(fireworks_points), fireworks_points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(fireworks_points), sizeof(fireworks_colors), fireworks_colors);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(fireworks_points)+sizeof(fireworks_colors), sizeof(fireworks_velocities), fireworks_velocities);
    
    
    // Load shaders and create a shader program (to be used in display())
    program = InitShader("vshader42.glsl", "fshader53.glsl");
    program2 = InitShader("vshader53.glsl", "fshader53.glsl");
    programfire=InitShader("vfire.glsl", "ffire.glsl");
    
    
    glEnable( GL_DEPTH_TEST );
    glClearColor(0.529, 0.807, 0.92, 0.0);
    glLineWidth(2.0);
}
//----------------------------------------------------------------------------
// drawObj(buffer, num_vertices):
//   draw the object that is associated with the vertex buffer object "buffer"
//   and has "num_vertices" vertices.
//
void drawObj(GLuint buffer, int num_vertices)
{
    
    
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    
    
    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation(program, "vPosition");
    glUniform1i(glGetUniformLocation(program, "fogFlag"), fogFlag);
    glUniform1i(glGetUniformLocation(program, "texture_2D"), 0);
    glUniform1i(glGetUniformLocation(program, "texture_1D"), 1);
    glUniform1i(glGetUniformLocation(program, "textureFlag"), textureFlag);
    glUniform1i(glGetUniformLocation(program, "draw"), draw);
    glUniform1i(glGetUniformLocation(program,"upLatticeFlag"),upLatticeFlag);
    glUniform1i(glGetUniformLocation(program,"latticeFlag"),latticeFlag);
    
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(0) );
    
    GLuint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(point4) * num_vertices) );
    // the offset is the (total) size of the previous vertex attribute array(s)
    
    
    //GLuint vTexCoord = glGetAttribLocation(program, "vTexCoord");
    //glEnableVertexAttribArray(vTexCoord);
    //glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point4)* num_vertices + sizeof(point4)* num_vertices));
    
    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
     (using the attributes specified in each enabled vertex attribute array) */
    glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    
    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);
}

void drawObj2(GLuint buffer, int num_vertices)
{
    
    
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    
    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation( program2, "vPosition" );
    glUniform1i(glGetUniformLocation(program2, "fogFlag"), fogFlag);
    glUniform1i(glGetUniformLocation(program2, "texture_2D"),0);
    glUniform1i(glGetUniformLocation(program2, "texture_1D"),1);
    glUniform1i(glGetUniformLocation(program2, "textureFlag"),textureFlag);
    glUniform1i(glGetUniformLocation(program2, "sphereTexFlag"),sphereTexFlag);
    glUniform1i(glGetUniformLocation(program2, "draw"),draw);
    glUniform1i(glGetUniformLocation(program2, "verticalFlag"),verticalFlag);
    glUniform1i(glGetUniformLocation(program2, "eyeSpaceFlag"),eyeSpaceFlag);
    glUniform1i(glGetUniformLocation(program2, "latticeFlag"),latticeFlag);
    glUniform1i(glGetUniformLocation(program2,"upLatticeFlag"),upLatticeFlag);

    
    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(0) );
    
    GLuint vNormal = glGetAttribLocation( program2, "vNormal" );
    glEnableVertexAttribArray( vNormal );
    glVertexAttribPointer( vNormal, 3, GL_FLOAT, GL_FALSE, 0,
                          BUFFER_OFFSET(sizeof(point4)*num_vertices));
    // the offset is the (total) size of the previous vertex attribute array(s)
    
    GLuint vTexCoord = glGetAttribLocation(program2, "vTexCoord");
    glEnableVertexAttribArray(vTexCoord);
    glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point4)* num_vertices + sizeof(vec3)* num_vertices));
    
    
    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
     (using the attributes specified in each enabled vertex attribute array) */
    glDrawArrays(GL_TRIANGLES, 0, num_vertices);
    
    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vNormal);
    glDisableVertexAttribArray(vTexCoord);
}

void drawFireworks(GLuint buffer, int num_vertices) {
       
    //--- Activate the vertex buffer object to be drawn ---//
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glUniform1f(glGetUniformLocation(programfire, "t"), time_New);
    
    /*----- Set up vertex attribute arrays for each vertex attribute -----*/
    GLuint vPosition = glGetAttribLocation(programfire, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    
    GLuint vColor = glGetAttribLocation(programfire, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point4)* num_vertices));
    
    GLuint vVelocity = glGetAttribLocation(programfire, "vVelocity");
    glEnableVertexAttribArray(vVelocity);
    glVertexAttribPointer(vVelocity, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(point4)* num_vertices * 2));
    
    /* Draw a sequence of geometric objs (triangles) from the vertex buffer
     (using the attributes specified in each enabled vertex attribute array) */
    glDrawArrays(GL_POINTS, 0, num_vertices);
    
    /*--- Disable each vertex attribute array being enabled ---*/
    glDisableVertexAttribArray(vVelocity);
    glDisableVertexAttribArray(vPosition);
    glDisableVertexAttribArray(vColor);
}




//-----------------------------------------------------------------------------
vec3 light_direction=vec3(0.1, 0.0, -1.0);
color4 global_ambient(1.0,1.0,1.0,1.0);

color4 light_ambient(0.0, 0.0, 0.0, 1.0);
color4 light_diffuse(0.8, 0.8, 0.8, 1.0);
color4 light_specular(0.2, 0.2, 0.2, 1.0);

float  material_shininess=125.0;

color4 material_ambient( 0.2, 0.2, 0.2, 1.0);
color4 material_diffuse( 1.0, 0.84, 0.0, 1.0 );
color4 material_specular(1.0, 0.84, 0.0, 1.0);

color4 ambient_product =  light_ambient* material_ambient;
color4 diffuse_product = light_diffuse * material_diffuse;
color4 specular_product = light_specular * material_specular;



void setUp_sphere(mat4 mv)
{
    glUniform3fv(glGetUniformLocation(program2, "SurfaceAmbient"), 1, material_ambient);
    glUniform3fv(glGetUniformLocation(program2, "LightDirection"), 1, light_direction);
    glUniform4fv(glGetUniformLocation(program2, "GlobalAmbient"), 1, global_ambient);
    glUniform4fv(glGetUniformLocation(program2, "AmbientProduct"), 1, ambient_product);
    glUniform4fv(glGetUniformLocation(program2, "DiffuseProduct"), 1, diffuse_product);
    glUniform4fv(glGetUniformLocation(program2, "SpecularProduct"),1, specular_product);
    glUniform1f(glGetUniformLocation(program2, "Shininess"), material_shininess);
}

color4 floor_diffuse(0.0, 1.0, 0.0, 1.0),floor_ambient(0.2, 0.2, 0.2, 1.0),floor_specular(0.0, 0.0, 0.0, 1.0);
color4 floor_ambient_product =  light_ambient* floor_ambient;
color4 floor_diffuse_product = light_diffuse * floor_diffuse;
color4 floor_specular_product = light_specular * floor_specular;

void setUp_floor(mat4 mv)
{
    glUniform3fv(glGetUniformLocation(program2, "SurfaceAmbient"), 1, floor_ambient);
    glUniform3fv(glGetUniformLocation(program2, "LightDirection"), 1, light_direction);
    glUniform4fv(glGetUniformLocation(program2, "GlobalAmbient"), 1, global_ambient);
    glUniform4fv(glGetUniformLocation(program2, "AmbientProduct"), 1, floor_ambient_product);
    glUniform4fv(glGetUniformLocation(program2, "DiffuseProduct"), 1, floor_diffuse_product);
    glUniform4fv(glGetUniformLocation(program2, "SpecularProduct"),1, floor_specular_product);
    glUniform1f(glGetUniformLocation(program2, "Shininess"), material_shininess);
}

//another light souce

color4 light_diffuse2(1.0, 1.0, 1.0, 1.0), light_specular2(1.0, 1.0, 1.0, 1.0), light_ambient2(0.0, 0.0, 0.0, 1.0);
vec4   light_source(-14.0, 12.0, -3.0, 1.0);
float constant_attenuation=2.0, linear_attenuation=0.01, quadratic_attenuation=0.001;
point4 light2_endpoint(-6.0, 0.0, -4.5, 1.0);
float exponent = 15.0;
float cutoff_angle = 20.0;
color4 floor_ambient_product2 =  light_ambient2* floor_ambient;
color4 floor_diffuse_product2 = light_diffuse2 * floor_diffuse;
color4 floor_specular_product2 = light_specular2 * floor_specular;

void setUp_point_floor(mat4 mv)
{
    vec4 point_position = mv * light_source;
    glUniform4fv(glGetUniformLocation(program2, "PointPosition"), 1, point_position);
    
    glUniform1i(glGetUniformLocation(program2, "Point_Source"), 1);
    
    glUniform4fv(glGetUniformLocation(program2, "PointAmbientProduct"), 1, floor_ambient_product2);
    glUniform4fv(glGetUniformLocation(program2, "PointDiffuseProduct"), 1, floor_diffuse_product2);
    glUniform4fv(glGetUniformLocation(program2, "PointSpecularProduct"), 1, floor_specular_product2);
    
    glUniform1f(glGetUniformLocation(program2, "Constant_Attenuation"), constant_attenuation);
    glUniform1f(glGetUniformLocation(program2, "Linear_Attenuation"), linear_attenuation);
    glUniform1f(glGetUniformLocation(program2, "Quadratic_Attenuation"), quadratic_attenuation);
}
void setUp_spot_floor(mat4 mv)
{
    vec4 point_position = mv * light_source;
    vec4 point_end_position = mv * light2_endpoint;
    glUniform4fv(glGetUniformLocation(program2, "PointPosition"), 1, point_position);
    glUniform1f(glGetUniformLocation(program2, "Exponent"), exponent);
    glUniform1f(glGetUniformLocation(program2, "CutOff"), cutoff_angle);
    glUniform4fv(glGetUniformLocation(program2, "PointEndPosition"), 1, point_end_position);
    
    glUniform1i(glGetUniformLocation(program2, "Point_Source"), 0);
    
    glUniform4fv(glGetUniformLocation(program2, "PointAmbientProduct"), 1, floor_ambient_product2);
    glUniform4fv(glGetUniformLocation(program2, "PointDiffuseProduct"), 1, floor_diffuse_product2);
    glUniform4fv(glGetUniformLocation(program2, "PointSpecularProduct"), 1, floor_specular_product2);
    
    glUniform1f(glGetUniformLocation(program2, "Constant_Attenuation"), constant_attenuation);
    glUniform1f(glGetUniformLocation(program2, "Linear_Attenuation"), linear_attenuation);
    glUniform1f(glGetUniformLocation(program2, "Quadratic_Attenuation"), quadratic_attenuation);
}

color4 ambient_product2 =  light_ambient2* material_ambient;
color4 diffuse_product2 = light_diffuse2 * material_diffuse;
color4 specular_product2 = light_specular2 * material_specular;

void setUp_point_sphere(mat4 mv)
{
    vec4 point_position = mv * light_source;
    glUniform4fv(glGetUniformLocation(program2, "PointPosition"), 1, point_position);
    
    glUniform1i(glGetUniformLocation(program2, "Point_Source"), 1);
    
    glUniform4fv(glGetUniformLocation(program2, "PointAmbientProduct"), 1, ambient_product2);
    glUniform4fv(glGetUniformLocation(program2, "PointDiffuseProduct"), 1, diffuse_product2);
    glUniform4fv(glGetUniformLocation(program2, "PointSpecularProduct"),1,specular_product2);
    
    glUniform1f(glGetUniformLocation(program2, "Constant_Attenuation"), constant_attenuation);
    glUniform1f(glGetUniformLocation(program2, "Linear_Attenuation"), linear_attenuation);
    glUniform1f(glGetUniformLocation(program2, "Quadratic_Attenuation"), quadratic_attenuation);
}
void setUp_spot_sphere(mat4 mv)
{
    vec4 point_position = mv * light_source;
    vec4 point_end_position = mv * light2_endpoint;
    glUniform4fv(glGetUniformLocation(program2, "PointPosition"), 1, point_position);
    glUniform1f(glGetUniformLocation(program2, "Exponent"), exponent);
    glUniform1f(glGetUniformLocation(program2, "CutOff"), cutoff_angle);
    glUniform4fv(glGetUniformLocation(program2, "PointEndPosition"), 1, point_end_position);
    
    glUniform1i(glGetUniformLocation(program2, "Point_Source"), 0);
    
    glUniform4fv(glGetUniformLocation(program2, "PointAmbientProduct"), 1, ambient_product2);
    glUniform4fv(glGetUniformLocation(program2, "PointDiffuseProduct"), 1, diffuse_product2);
    glUniform4fv(glGetUniformLocation(program2, "PointSpecularProduct"), 1,specular_product2);
    
    glUniform1f(glGetUniformLocation(program2, "Constant_Attenuation"), constant_attenuation);
    glUniform1f(glGetUniformLocation(program2, "Linear_Attenuation"), linear_attenuation);
    glUniform1f(glGetUniformLocation(program2, "Quadratic_Attenuation"), quadratic_attenuation);
}


//----------------------------------------------------------------------------
void display( void )
{
    GLuint  model_view,model_view2,model_view3;  // model-view matrix uniform shader variable location
    GLuint  projection,projection2,projection3;  // projection matrix uniform shader variable location
    
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    
    glUseProgram(program); // Use the shader program
    
    model_view = glGetUniformLocation(program, "model_view" );
    projection = glGetUniformLocation(program, "projection" );
    
    glUseProgram(program2);// Use the shader program2
    
    model_view2 = glGetUniformLocation(program2, "model_view");
    projection2 = glGetUniformLocation(program2, "projection");
    
    glUseProgram(programfire);
    model_view3 = glGetUniformLocation(programfire, "model_view");
    projection3 = glGetUniformLocation(programfire, "projection");

    
    /*---  Set up and pass on Projection matrix to the shader ---*/
    mat4  p = Perspective(fovy, aspect, zNear, zFar);
    
    
    glUseProgram(program);  glUniformMatrix4fv(projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major
    
    glUseProgram(program2);	glUniformMatrix4fv(projection2, 1, GL_TRUE, p);
    
    glUseProgram(programfire);	glUniformMatrix4fv(projection3, 1, GL_TRUE, p);
    
    /*---  Set up and pass on Model-View matrix to the shader ---*/
    // eye is a global variable of vec4 set to init_eye and updated by keyboard()
    vec4    at(0.0, 0.0, 0.0, 1.0);
    vec4    up(0.0, 1.0, 0.0, 0.0);
    
    glUseProgram(program);
    mat4  mv = LookAt(eye, at, up);
    glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawObj(coordinate_buffer, 9);
    
    mat3 normal_matrix;
    
    if (LightingFlag==0){
        mv = LookAt(eye, at, up)*Translate(move+A)* Rotate(angle, tw.z, 0.0, -tw.x) * Mrotate;
        glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);
        if (WireFramFlag==1)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        draw=2;
        drawObj(sphere_buffer, sphere_numVertices);
        draw=0;
        if (shadowFlag==0)
        {
            mv = LookAt(eye, at, up);
            glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            draw=1;
            drawObj(floor_buffer, floor_NumVertices);
            draw=0;
        }
        else
        {
            glDepthMask(GL_FALSE);
            mv = LookAt(eye, at, up);
            glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            draw=1;
            drawObj(floor_buffer, floor_NumVertices);
            draw=0;
            glDepthMask(GL_TRUE);
            mat4 shadow = mat4(vec4(12, 14, 0, 0),
                               vec4(0, 0, 0, 0),
                               vec4(0, 3, 12, 0),
                               vec4(0, -1, 0, 12));
            mv = LookAt(eye, at, up)*shadow*Translate(move+A)* Rotate(angle, tw.z, 0.0, -tw.x) * Mrotate;
            glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            drawObj(shadow_buffer, sphere_numVertices);//draw the shadow
            
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            mv = LookAt(eye, at, up);
            glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            draw=1;
            drawObj(floor_buffer, floor_NumVertices);
            draw=0;
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        }
    }else
    {
        if (WireFramFlag==1)
        {mv = LookAt(eye, at, up)*Translate(move+A)* Rotate(angle, tw.z, 0.0, -tw.x) * Mrotate;
        glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            drawObj(sphere_buffer, sphere_numVertices);}
    
        else
        {glUseProgram(program2);
        mv = LookAt(eye, at, up);
        setUp_sphere(mv);
        if(spotLightFlag==1)
            setUp_spot_sphere(mv);
        else
            setUp_point_sphere(mv);
        mv = LookAt(eye, at, up)*Translate(move+A)* Rotate(angle, tw.z, 0.0, -tw.x) * Mrotate;
        
        normal_matrix = NormalMatrix(mv, 1);
        glUniformMatrix3fv(glGetUniformLocation(program2, "Normal_Matrix"), 1, GL_TRUE, normal_matrix);
        glUniformMatrix4fv(model_view2, 1, GL_TRUE, mv);
        
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        
            if (smoothFlag==1) {
                draw=2;
                drawObj2(smooth_sphere_buffer, sphere_numVertices);
                draw=0;
            }
            else
            {
                draw=2;
                drawObj2(flat_sphere_buffer, sphere_numVertices);
                draw=0;
            }
            }

        if (shadowFlag==1)
        {
            
            glUseProgram(program2);
            
            glDepthMask(GL_FALSE);
            
            mv = LookAt(eye, at, up);
            setUp_floor(mv);
            if(spotLightFlag==1)
                setUp_spot_sphere(mv);
            else
                setUp_point_floor(mv);
            normal_matrix = NormalMatrix(mv, 1);
            glUniformMatrix3fv(glGetUniformLocation(program2, "Normal_Matrix"), 1, GL_TRUE, normal_matrix);
            glUniformMatrix4fv(model_view2, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
            if (floorFlag == 1) // Filled floor
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            else              // Wireframe floor
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            draw=1;
            drawObj2(shaded_floor_buffer, floor_NumVertices);  // draw the floor
            draw=0;
            glUseProgram(program);
            mat4 shadow = mat4(vec4(12, 14, 0, 0),
                               vec4(0, 0, 0, 0),
                               vec4(0, 3, 12, 0),
                               vec4(0, -1, 0, 12));
            mv = LookAt(eye, at, up)*shadow*Translate(move+A)* Rotate(angle, tw.z, 0.0, -tw.x) * Mrotate;
            glUniformMatrix4fv(model_view, 1, GL_TRUE, mv);
            if (WireFramFlag==1)
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            else
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            
            
            if (blendingShadowFlag==1)
                glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            draw=2;
            drawObj(shadow_buffer, sphere_numVertices);//draw the shadow
            draw=0;
            glDisable(GL_BLEND);
            
            glDepthMask(GL_TRUE);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            
            
            
            
            glUseProgram(program2);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            mv = LookAt(eye, at, up);
            setUp_floor(mv);
            if(spotLightFlag==1)
                setUp_spot_sphere(mv);
            else
                setUp_point_floor(mv);
            normal_matrix = NormalMatrix(mv, 1);
            glUniformMatrix3fv(glGetUniformLocation(program2, "Normal_Matrix"), 1, GL_TRUE, normal_matrix);
            glUniformMatrix4fv(model_view2, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
            if (floorFlag == 1) // Filled floor
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            else              // Wireframe floor
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            draw=1;
            drawObj2(shaded_floor_buffer, floor_NumVertices);  // draw the floor
            draw=0;            
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            
        }else
        {
            glUseProgram(program2);
            mv = LookAt(eye, at, up);
            setUp_floor(mv);
            if(spotLightFlag==1)
                setUp_spot_sphere(mv);
            else
                setUp_point_floor(mv);
            normal_matrix = NormalMatrix(mv, 1);
            glUniformMatrix3fv(glGetUniformLocation(program2, "Normal_Matrix"), 1, GL_TRUE, normal_matrix);
            glUniformMatrix4fv(model_view2, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
            
            
            if (floorFlag == 1) // Filled floor
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            else              // Wireframe floor
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            draw=1;
            drawObj2(shaded_floor_buffer, floor_NumVertices);  // draw the floor
            draw=0;
        }
    }
    
    if (fireworkFlag==1) {
        mv = LookAt(eye, at, up);
        glUseProgram(programfire);
        glUniformMatrix4fv(model_view3, 1, GL_TRUE, mv);
        drawFireworks(firework_buffer, N);
    }
    
    glutSwapBuffers();
}
//---------------------------------------------------------------------------
void idle (void)
{
    
    AB=AB/sqrt(AB.x*AB.x+AB.y*AB.y+AB.z*AB.z);
    BC=BC/sqrt(BC.x*BC.x+BC.y*BC.y+BC.z*BC.z);
    CA=CA/sqrt(CA.x*CA.x+CA.y*CA.y+CA.z*CA.z);
    if (2*M_PI*sum/360<sqrt(9+64))
    { if (zone==3)
    {   Mrotate=Rotate(angle, tw.z, 0.0, -tw.x)*Mrotate;
        angle=0;
    }
        zone=1;
        tw=AB;
        move=move+(2*M_PI/360*AB);}
    else if (2*M_PI*sum/360<sqrt(16+81)+sqrt(9+64))
    {
        if (zone==1)
        {Mrotate=Rotate(angle, tw.z, 0.0, -tw.x)*Mrotate;
            angle=0;}
        zone=2;
        tw=BC;
        move=move+(2*M_PI/360*BC);}
    else if (2*M_PI*sum/360<sqrt(16+81)+sqrt(49+1)+sqrt(9+64))
    {
        if (zone==2)
        {Mrotate=Rotate(angle, tw.z, 0.0, -tw.x)*Mrotate;
            angle=0;}
        zone=3;
        tw=CA;
        move=move+(2*M_PI/360*CA);}
    else sum=sum-(sqrt(16+81)+sqrt(49+1)+sqrt(9+64))*360/(2*M_PI);
    //angle += 0.02;
    
    angle += 1.0;    //YJC: change this value to adjust the cube rotation speed.
    sum +=1.0;
    
    time_Old = (float)glutGet(GLUT_ELAPSED_TIME);
    time_New = fmod(time_Old - time_Sub, time_Max);

    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void keyboard(unsigned char key, int x, int y)
{
    switch(key) {
        case 033: // Escape Key
        case 'q': case 'Q':
            exit( EXIT_SUCCESS );
            break;
            
        case 'X': eye[0] += 1.0; break;
        case 'x': eye[0] -= 1.0; break;
        case 'Y': eye[1] += 1.0; break;
        case 'y': eye[1] -= 1.0; break;
        case 'Z': eye[2] += 1.0; break;
        case 'z': eye[2] -= 1.0; break;
            
        case 'b': case 'B': // Toggle between animation and non-animation
            animationFlag = 1 -  animationFlag;
            avaliable = true;
            if (animationFlag == 1) glutIdleFunc(NULL);
            else                    glutIdleFunc(idle);
            break;
            
        case 'c': case 'C': // Toggle between filled and wireframe cube
            cubeFlag = 1 -  cubeFlag;
            break;
            
        case 'f': case 'F': // Toggle between filled and wireframe floor
            floorFlag = 1 -  floorFlag;
            break;
            
        case ' ':  // reset to initial viewer/eye position
            eye = init_eye;
            break;
        case 'v': case 'V':
            verticalFlag=1;
            break;
        case 's': case 'S':
            verticalFlag=0;
            break;
        case 'o': case 'O':
            eyeSpaceFlag=0;
            break;
        case 'e': case 'E':
            eyeSpaceFlag=1;
            break;
        case 'u': case 'U':
            upLatticeFlag=1;
            break;
        case 't': case 'T':
            upLatticeFlag=0;
            break;
        case 'l': case 'L':
            latticeFlag=1-latticeFlag;
            break;

            
            
    }
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void menu(int id)
{
    switch(id)
    {
        case 0: eye = init_eye;
            break;
        case 1: exit(0);
            break;
    }
    glutPostRedisplay();
}

void wireframe_menu(int index)
{
    WireFramFlag= (index == 1)?1:0;
    display();
}
void shadow_menu(int index){
    shadowFlag = (index == 1)?1:0;
    display();
};

/* shade menu handler
 1) Flat shade
 2) Smooth shade
 */
void shade_menu(int index){
    smoothFlag = (index == 1)?1:0;
    display();
};

/* lighting menu handler
 1) Turn on lighting effect
 2) Turn off lighting effect
 */
void lighting_menu(int index){
    LightingFlag = (index == 1)?1:0;
    display();
};

/* spotlight menu handler
 1) Spotlight
 2) Point light
 */
void spotlight_menu(int index){
    spotLightFlag = (index == 1)?1:0;
    display();
};


void  fog_menu(int index)
{
    fogFlag=index;
    display();
}

void  blending_menu(int index)
{
    blendingShadowFlag=index;
    display();
}

void texture_menu(int index)
{
    textureFlag=index;
    display();
}
void sphere_texture_menu(int index)
{
    sphereTexFlag=index;
    display();
}

void firework_menu(int index)
{
    switch (index) {
        case 0:
            fireworkFlag = 0;
            break;
        case 1:
            if (fireworkFlag == 0) {
                fireworkFlag = 1;
                time_Old = (float)glutGet(GLUT_ELAPSED_TIME);
                time_Sub = time_Old;
                time_New = 0.0f;
            }
            break;
    }
    display();
}

//------------------------------------------------------------------


void addMenu(){
    int shadow = glutCreateMenu(shadow_menu);
    glutAddMenuEntry("No", 0);
    glutAddMenuEntry("Yes", 1);
    
    int shade = glutCreateMenu(shade_menu);
    glutAddMenuEntry("flat shading", 0);
    glutAddMenuEntry("smooth shading", 1);
    
    int lighting = glutCreateMenu(lighting_menu);
    glutAddMenuEntry("No", 0);
    glutAddMenuEntry("Yes", 1);
    
    int spotlight = glutCreateMenu(spotlight_menu);
    glutAddMenuEntry("Spot light", 1);
    glutAddMenuEntry("Point light", 0);
    
    int wireframe=glutCreateMenu(wireframe_menu);
    glutAddMenuEntry("Yes", 1);
    glutAddMenuEntry("No", 0);
    
    int fog=glutCreateMenu(fog_menu);
    glutAddMenuEntry("no fog", 0);
    glutAddMenuEntry("linear", 1);
    glutAddMenuEntry("exponential", 2);
    glutAddMenuEntry("exponential square", 3);
    
    int blending= glutCreateMenu(blending_menu);
    glutAddMenuEntry("No", 0);
    glutAddMenuEntry("Yes", 1);
    
    int texture=glutCreateMenu(texture_menu);
    glutAddMenuEntry("No", 0);
    glutAddMenuEntry("Yes", 1);
    
    int sphereTex=glutCreateMenu(sphere_texture_menu);
    glutAddMenuEntry("No", 0);
    glutAddMenuEntry("Yes - Contour Lines", 1);
    glutAddMenuEntry("Yes - Checkerboard", 2);

    int firework=glutCreateMenu(firework_menu);
    glutAddMenuEntry("No", 0);
    glutAddMenuEntry("Yes", 1);
    
    glutCreateMenu(menu);
    glutAddMenuEntry("Default View Point", 0);
    
    
    glutAddMenuEntry("Quit", 1);
    glutAddSubMenu("Wire Frame Sphere", wireframe);
    glutAddSubMenu("Enable Lighting", lighting);
    glutAddSubMenu("Shadow",shadow);
    glutAddSubMenu("Shading", shade);
    glutAddSubMenu("Lighting", spotlight);
    glutAddSubMenu("Fog Options", fog);
    glutAddSubMenu("Blending Shadow", blending);
    glutAddSubMenu("Texture Mapped Ground", texture);
    glutAddSubMenu("Texture Mapped Sphere", sphereTex);
    glutAddSubMenu("Firework", firework);

    
    glutAttachMenu(GLUT_LEFT_BUTTON);
}
void myMouse(int button, int state, int x,int y)
{
    if (button == GLUT_RIGHT_BUTTON&&state== GLUT_DOWN&&avaliable==true)
        animationFlag = 1 -  animationFlag;
    if (animationFlag == 1) glutIdleFunc(NULL);
    else                    glutIdleFunc(idle);
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
void reshape(int width, int height)
{
    glViewport(0, 0, width, height);
    aspect = (GLfloat) width  / (GLfloat) height;
    glutPostRedisplay();
}
//----------------------------------------------------------------------------
int main(int argc, char **argv)
{   int err;
    filename = chooseFile();
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(512, 512);
    // glutInitContextVersion(3, 2);
    // glutInitContextProfile(GLUT_CORE_PROFILE);
    glutCreateWindow("Assignment4");
    
    /* Call glewInit() and error checking */
#ifndef __APPLE__
    err = glewInit();
    if (GLEW_OK != err)
    { printf("Error: glewInit failed: %s\n", (char*) glewGetErrorString(err)); 
        exit(1);
    }
#endif
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(NULL);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(myMouse);
    addMenu();
    init();
    glutMainLoop();
    return 0;
}
