#include "prelude.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define BUFFER_OFFSET(i) ((char*)NULL + (i))

GLuint gProgram[12];
int gWidth, gHeight;
float posx = 0.0;

float initial_block_z = -45.0;
float initial_block_z2 = -90.0;
float initial_block_z3 = -135.0;
float block_z = -45.0;
float block_z2 = -90.0;
float block_z3 = -135.0;

float plane_movement = 0.0;
float speed = 0.0;
float change_speed = 0.05;
float jump_height = 0.5;
float rotation = 0.0;
bool jump = true;
bool hit_top = false;
bool dead = false;
bool left = false;
bool right = false;
std::vector<int> vertex_counts;

struct Character {
    GLuint TextureID;   // ID handle of the glyph texture
    glm::ivec2 Size;    // Size of glyph
    glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
    GLuint Advance;    // Horizontal offset to advance to next glyph
};

std::map<GLchar, Character> Characters;

GLint modelingMatrixLoc[12];
GLint viewingMatrixLoc[12];
GLint projectionMatrixLoc[12];
GLint eyePosLoc[12];

GLuint vao0_got;
int zero_size;
GLuint vao1_got;
int first_size;
GLuint vao2_got;
int second_size;
GLuint vao3_got;
int third_size;
GLuint vao4_got;
int fourth_size;
GLuint vao5_got;
int fifth_size;
GLuint vao6_got;
int sixth_size;
GLuint vao7_got;
int seventh_size;
GLuint vao8_got;
int eigth_size;
GLuint vao9_got;
int ninth_size;
GLuint vao10_got;
int tenth_size;
GLuint vao11_got;
int eleventh_size;


int score = 0;
bool spin = false;
int spin_deg = 0;

int yel_poz1 = rand() % 3;
int yel_poz2 = rand() % 3;
int yel_poz3 = rand() % 3;

float red11 = 0.0;
float red12 = 0.0;
float yel1 = 0.0;

float red21 = 0.0;
float red22 = 0.0;
float yel2 = 0.0;

float red31 = 0.0;
float red32 = 0.0;
float yel3 = 0.0;

int nearest_row = 1;

float left_cube_max_x = -2.0;
float left_cube_min_x = -4.0;

float mid_cube_max_x = 1.0;
float mid_cube_min_x = -1.0;

float right_cube_max_x = 4.0;
float right_cube_min_x = 2.0;

float nearest_row_max_z = 0.0;
float nearest_row_min_z = 0.0;

float bunny_x_min = -1.8938;
float bunny_x_max = 1.22018;

float bunny_z_min = -1.23747;
float bunny_z_max = 1.17599;

bool inside = false;

int width, height, nrChannels;
GLuint renderer_id;
GLuint sampler_id;


glm::mat4 projectionMatrix;
glm::mat4 viewingMatrix;
glm::mat4 modelingMatrix;
glm::vec3 eyePos(0, 2, -10);

int activeProgramIndex = 0;

struct Vertex
{
	Vertex(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
	GLfloat x, y, z;
};

struct Texture
{
	Texture(GLfloat inU, GLfloat inV) : u(inU), v(inV) { }
	GLfloat u, v;
};

struct Normal
{
	Normal(GLfloat inX, GLfloat inY, GLfloat inZ) : x(inX), y(inY), z(inZ) { }
	GLfloat x, y, z;
};

struct Face
{
	Face(int v[], int t[], int n[]) {
		vIndex[0] = v[0];
		vIndex[1] = v[1];
		vIndex[2] = v[2];
		tIndex[0] = t[0];
		tIndex[1] = t[1];
		tIndex[2] = t[2];
		nIndex[0] = n[0];
		nIndex[1] = n[1];
		nIndex[2] = n[2];
	}
	GLuint vIndex[3], tIndex[3], nIndex[3];
};

std::vector<Vertex> gVertices;
std::vector<Texture> gTextures;
std::vector<Normal> gNormals;
std::vector<Face> gFaces;

GLuint gVertexAttribBuffer, gIndexBuffer;
GLint gInVertexLoc, gInNormalLoc;
int gVertexDataSizeInBytes, gNormalDataSizeInBytes;

bool ParseObj(const std::string& fileName)
{
	std::fstream myfile;

	// Open the input
	myfile.open(fileName.c_str(), std::ios::in);

	if (myfile.is_open())
	{
		std::string curLine;

		while (getline(myfile, curLine))
		{
			std::stringstream str(curLine);
			GLfloat c1, c2, c3;
			GLuint index[9];
			std::string tmp;

			if (curLine.length() >= 2)
			{
				if (curLine[0] == 'v')
				{
					if (curLine[1] == 't') // texture
					{
						str >> tmp; // consume "vt"
						str >> c1 >> c2;
						gTextures.push_back(Texture(c1, c2));
					}
					else if (curLine[1] == 'n') // normal
					{
						str >> tmp; // consume "vn"
						str >> c1 >> c2 >> c3;
						gNormals.push_back(Normal(c1, c2, c3));
					}
					else // vertex
					{
						str >> tmp; // consume "v"
						str >> c1 >> c2 >> c3;
						gVertices.push_back(Vertex(c1, c2, c3));
					}
				}
				else if (curLine[0] == 'f') // face
				{
					str >> tmp; // consume "f"
					char c;
					int vIndex[3], nIndex[3], tIndex[3];
					str >> vIndex[0]; str >> c >> c; // consume "//"
					str >> nIndex[0];
					str >> vIndex[1]; str >> c >> c; // consume "//"
					str >> nIndex[1];
					str >> vIndex[2]; str >> c >> c; // consume "//"
					str >> nIndex[2];

					assert(vIndex[0] == nIndex[0] &&
						vIndex[1] == nIndex[1] &&
						vIndex[2] == nIndex[2]); // a limitation for now

					// make indices start from 0
					for (int c = 0; c < 3; ++c)
					{
						vIndex[c] -= 1;
						nIndex[c] -= 1;
						tIndex[c] -= 1;
					}

					gFaces.push_back(Face(vIndex, tIndex, nIndex));
				}
				else
				{
					std::cout << "Ignoring unidentified line in obj file: " << curLine << std::endl;
				}
			}

			//data += curLine;
			if (!myfile.eof())
			{
				//data += "\n";
			}
		}

		myfile.close();
	}
	else
	{
		return false;
	}

	/*
	for (int i = 0; i < gVertices.size(); ++i)
	{
		Vector3 n;

		for (int j = 0; j < gFaces.size(); ++j)
		{
			for (int k = 0; k < 3; ++k)
			{
				if (gFaces[j].vIndex[k] == i)
				{
					// face j contains vertex i
					Vector3 a(gVertices[gFaces[j].vIndex[0]].x,
							  gVertices[gFaces[j].vIndex[0]].y,
							  gVertices[gFaces[j].vIndex[0]].z);

					Vector3 b(gVertices[gFaces[j].vIndex[1]].x,
							  gVertices[gFaces[j].vIndex[1]].y,
							  gVertices[gFaces[j].vIndex[1]].z);

					Vector3 c(gVertices[gFaces[j].vIndex[2]].x,
							  gVertices[gFaces[j].vIndex[2]].y,
							  gVertices[gFaces[j].vIndex[2]].z);

					Vector3 ab = b - a;
					Vector3 ac = c - a;
					Vector3 normalFromThisFace = (ab.cross(ac)).getNormalized();
					n += normalFromThisFace;
				}

			}
		}

		n.normalize();

		gNormals.push_back(Normal(n.x, n.y, n.z));
	}*/


	assert(gVertices.size() == gNormals.size());
    if(vertex_counts.size()>0){
        vertex_counts.push_back(gFaces.size()*3-vertex_counts[vertex_counts.size()-1]);
    }else{
        vertex_counts.push_back(gFaces.size()*3);
    }

	return true;
}

bool ReadDataFromFile(
	const std::string& fileName, ///< [in]  Name of the shader file
	std::string& data)     ///< [out] The contents of the file
{
	std::fstream myfile;

	// Open the input
	myfile.open(fileName.c_str(), std::ios::in);

	if (myfile.is_open())
	{
		std::string curLine;

		while (getline(myfile, curLine))
		{
			data += curLine;
			if (!myfile.eof())
			{
				data += "\n";
			}
		}

		myfile.close();
	}
	else
	{
		return false;
	}

	return true;
}

GLuint createVS(const char* shaderName)
{
	std::string shaderSource;

	std::string filename(shaderName);
	if (!ReadDataFromFile(filename, shaderSource))
	{
		std::cout << "Cannot find file name: " + filename << std::endl;
		exit(-1);
	}

	GLint length = shaderSource.length();
	const GLchar* shader = (const GLchar*)shaderSource.c_str();

	GLuint vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, &shader, &length);
	glCompileShader(vs);

	char output[1024] = { 0 };
	glGetShaderInfoLog(vs, 1024, &length, output);
	printf("VS compile log: %s\n", output);

	return vs;
}

GLuint createFS(const char* shaderName)
{
	std::string shaderSource;

	std::string filename(shaderName);
	if (!ReadDataFromFile(filename, shaderSource))
	{
		std::cout << "Cannot find file name: " + filename << std::endl;
		exit(-1);
	}

	GLint length = shaderSource.length();
	const GLchar* shader = (const GLchar*)shaderSource.c_str();

	GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fs, 1, &shader, &length);
	glCompileShader(fs);

	char output[1024] = { 0 };
	glGetShaderInfoLog(fs, 1024, &length, output);
	printf("FS compile log: %s\n", output);

	return fs;
}

void initTexture(){

    unsigned char *data;
    stbi_set_flip_vertically_on_load(1);
    data = stbi_load("sky.jpg",&width, &height, &nrChannels, 0);

    glGenSamplers(1, &sampler_id);
    glGenTextures(1, &renderer_id);
    
    glBindSampler(0, sampler_id);
    glBindTexture(GL_TEXTURE_2D, renderer_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	
    //glBindTexture(GL_TEXTURE_2D, 0);

    if(data){
        stbi_image_free(data);
    }
}

void deleteTexture(){

    glDeleteTextures(1, &renderer_id);
}

void initShaders()
{
	// Create the programs

	gProgram[0] = glCreateProgram();
	gProgram[1] = glCreateProgram();
    gProgram[2] = glCreateProgram();
    gProgram[3] = glCreateProgram();
    gProgram[4] = glCreateProgram();
    gProgram[5] = glCreateProgram();
    gProgram[6] = glCreateProgram();
    gProgram[7] = glCreateProgram();
    gProgram[8] = glCreateProgram();
    gProgram[9] = glCreateProgram();
    gProgram[10] = glCreateProgram();
    gProgram[11] = glCreateProgram();

	// Create the shaders for both programs

    GLuint vs0 = createVS("sky_vs.glsl");
    GLuint fs0 = createFS("sky_fs.glsl");

    GLuint vs1 = createVS("vert_text.glsl");
    GLuint fs1 = createFS("frag_text.glsl");

    GLuint vs2 = createVS("vert2.glsl");
    GLuint fs2 = createFS("frag2.glsl");

    GLuint vs3 = createVS("red_cube_vs.glsl");
    GLuint fs3 = createFS("red_cube_fs.glsl");

    GLuint vs4 = createVS("red_cube_vs.glsl");
    GLuint fs4 = createFS("red_cube_fs.glsl");

    GLuint vs5 = createVS("yellow_cube_vs.glsl");
    GLuint fs5 = createFS("yellow_cube_fs.glsl");

    GLuint vs6 = createVS("red_cube_vs.glsl");
    GLuint fs6 = createFS("red_cube_fs.glsl");

    GLuint vs7 = createVS("red_cube_vs.glsl");
    GLuint fs7 = createFS("red_cube_fs.glsl");

    GLuint vs8 = createVS("yellow_cube_vs.glsl");
    GLuint fs8 = createFS("yellow_cube_fs.glsl");

    GLuint vs9 = createVS("red_cube_vs.glsl");
    GLuint fs9 = createFS("red_cube_fs.glsl");

    GLuint vs10 = createVS("red_cube_vs.glsl");
    GLuint fs10 = createFS("red_cube_fs.glsl");

    GLuint vs11 = createVS("yellow_cube_vs.glsl");
    GLuint fs11 = createFS("yellow_cube_fs.glsl");




    // Attach the shaders to the programs

	glAttachShader(gProgram[0], vs0);
	glAttachShader(gProgram[0], fs0);

	glAttachShader(gProgram[1], vs1);
	glAttachShader(gProgram[1], fs1);

    glAttachShader(gProgram[2], vs2);
    glAttachShader(gProgram[2], fs2);

    glAttachShader(gProgram[3], vs3);
    glAttachShader(gProgram[3], fs3);

    glAttachShader(gProgram[4], vs4);
    glAttachShader(gProgram[4], fs4);

    glAttachShader(gProgram[5], vs5);
    glAttachShader(gProgram[5], fs5);

    glAttachShader(gProgram[6], vs6);
    glAttachShader(gProgram[6], fs6);

    glAttachShader(gProgram[7], vs7);
    glAttachShader(gProgram[7], fs7);

    glAttachShader(gProgram[8], vs8);
    glAttachShader(gProgram[8], fs8);

    glAttachShader(gProgram[9], vs9);
    glAttachShader(gProgram[9], fs9);

    glAttachShader(gProgram[10], vs10);
    glAttachShader(gProgram[10], fs10);

    glAttachShader(gProgram[11], vs11);
    glAttachShader(gProgram[11], fs11);
	// Link the programs

	glLinkProgram(gProgram[0]);
	GLint status;
	glGetProgramiv(gProgram[0], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		std::cout << "Program link failed" << std::endl;
		exit(-1);
	}

	glLinkProgram(gProgram[1]);
	glGetProgramiv(gProgram[1], GL_LINK_STATUS, &status);

	if (status != GL_TRUE)
	{
		std::cout << "Program link failed" << std::endl;
		exit(-1);
	}

    glLinkProgram(gProgram[2]);
    glGetProgramiv(gProgram[2], GL_LINK_STATUS, &status);

    if (status != GL_TRUE)
    {
        std::cout << "Program link failed" << std::endl;
        exit(-1);
    }

    glLinkProgram(gProgram[3]);
    glGetProgramiv(gProgram[3], GL_LINK_STATUS, &status);

    if (status != GL_TRUE)
    {
        std::cout << "Program link failed" << std::endl;
        exit(-1);
    }

    glLinkProgram(gProgram[4]);
    glGetProgramiv(gProgram[4], GL_LINK_STATUS, &status);

    if (status != GL_TRUE)
    {
        std::cout << "Program link failed" << std::endl;
        exit(-1);
    }

    glLinkProgram(gProgram[5]);
    glGetProgramiv(gProgram[5], GL_LINK_STATUS, &status);

    if (status != GL_TRUE)
    {
        std::cout << "Program link failed" << std::endl;
        exit(-1);
    }

    glLinkProgram(gProgram[6]);
    glGetProgramiv(gProgram[6], GL_LINK_STATUS, &status);

    if (status != GL_TRUE)
    {
        std::cout << "Program link failed" << std::endl;
        exit(-1);
    }

    glLinkProgram(gProgram[7]);
    glGetProgramiv(gProgram[7], GL_LINK_STATUS, &status);

    if (status != GL_TRUE)
    {
        std::cout << "Program link failed" << std::endl;
        exit(-1);
    }

    glLinkProgram(gProgram[8]);
    glGetProgramiv(gProgram[8], GL_LINK_STATUS, &status);

    if (status != GL_TRUE)
    {
        std::cout << "Program link failed" << std::endl;
        exit(-1);
    }

    glLinkProgram(gProgram[9]);
    glGetProgramiv(gProgram[9], GL_LINK_STATUS, &status);

    if (status != GL_TRUE)
    {
        std::cout << "Program link failed" << std::endl;
        exit(-1);
    }

    glLinkProgram(gProgram[10]);
    glGetProgramiv(gProgram[10], GL_LINK_STATUS, &status);

    if (status != GL_TRUE)
    {
        std::cout << "Program link failed" << std::endl;
        exit(-1);
    }

    glLinkProgram(gProgram[11]);
    glGetProgramiv(gProgram[11], GL_LINK_STATUS, &status);

    if (status != GL_TRUE)
    {
        std::cout << "Program link failed" << std::endl;
        exit(-1);
    }
	glBindAttribLocation(gProgram[0], 1, "i_texCoord");

	// Get the locations of the uniform variables from both programs

	for (int i = 0; i < 12; ++i)
	{
		modelingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "modelingMatrix");
		viewingMatrixLoc[i] = glGetUniformLocation(gProgram[i], "viewingMatrix");
		projectionMatrixLoc[i] = glGetUniformLocation(gProgram[i], "projectionMatrix");
		eyePosLoc[i] = glGetUniformLocation(gProgram[i], "eyePos");
	}
}

void initVBO(GLuint &vao)
{

    glGenVertexArrays(1, &vao);
    assert(vao > 0);
    glBindVertexArray(vao);
    std::cout << "vao = " << vao << std::endl;

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    assert(glGetError() == GL_NONE);

    glGenBuffers(1, &gVertexAttribBuffer);
    glGenBuffers(1, &gIndexBuffer);

    assert(gVertexAttribBuffer > 0 && gIndexBuffer > 0);

    glBindBuffer(GL_ARRAY_BUFFER, gVertexAttribBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gIndexBuffer);

    gVertexDataSizeInBytes = gVertices.size() * 3 * sizeof(GLfloat);
    gNormalDataSizeInBytes = gNormals.size() * 3 * sizeof(GLfloat);
    int indexDataSizeInBytes = gFaces.size() * 3 * sizeof(GLuint);
    int textureDataSizeInBytes = gTextures.size() * 2 * sizeof(GLfloat);
    GLfloat* vertexData = new GLfloat[gVertices.size() * 3];
    GLfloat* normalData = new GLfloat[gNormals.size() * 3];
    GLfloat* textureData = new GLfloat[gTextures.size() * 2];
    GLuint* indexData = new GLuint[gFaces.size() * 3];

    float minX = 1e6, maxX = -1e6;
    float minY = 1e6, maxY = -1e6;
    float minZ = 1e6, maxZ = -1e6;

    for (int i = 0; i < gVertices.size(); ++i)
    {
        vertexData[3 * i] = gVertices[i].x;
        vertexData[3 * i + 1] = gVertices[i].y;
        vertexData[3 * i + 2] = gVertices[i].z;

        minX = std::min(minX, gVertices[i].x);
        maxX = std::max(maxX, gVertices[i].x);
        minY = std::min(minY, gVertices[i].y);
        maxY = std::max(maxY, gVertices[i].y);
        minZ = std::min(minZ, gVertices[i].z);
        maxZ = std::max(maxZ, gVertices[i].z);
    }

    std::cout << "minX = " << minX << std::endl;
    std::cout << "maxX = " << maxX << std::endl;
    std::cout << "minY = " << minY << std::endl;
    std::cout << "maxY = " << maxY << std::endl;
    std::cout << "minZ = " << minZ << std::endl;
    std::cout << "maxZ = " << maxZ << std::endl;

    for (int i = 0; i < gTextures.size(); ++i)
    {
        textureData[2 * i] = gTextures[i].u;
        textureData[2 * i + 1] = gTextures[i].v;

    }
    for (int i = 0; i < gNormals.size(); ++i)
    {
        normalData[3 * i] = gNormals[i].x;
        normalData[3 * i + 1] = gNormals[i].y;
        normalData[3 * i + 2] = gNormals[i].z;
    }

    for (int i = 0; i < gFaces.size(); ++i)
    {
        indexData[3 * i] = gFaces[i].vIndex[0];
        indexData[3 * i + 1] = gFaces[i].vIndex[1];
        indexData[3 * i + 2] = gFaces[i].vIndex[2];
    }


    glBufferData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes + gNormalDataSizeInBytes, 0, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, gVertexDataSizeInBytes, vertexData);
    glBufferSubData(GL_ARRAY_BUFFER, gVertexDataSizeInBytes, gNormalDataSizeInBytes, normalData);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, textureDataSizeInBytes, textureData, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexDataSizeInBytes, indexData, GL_STATIC_DRAW);

    delete[] vertexData;
    delete[] textureData;
    delete[] normalData;
    delete[] indexData;

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(gVertexDataSizeInBytes));
}

void init()
{

	ParseObj("quad.obj");
    initVBO(vao0_got);
    zero_size = gFaces.size()*3;

    gVertices.clear();
    gFaces.clear();
    gNormals.clear();

    ParseObj("quad.obj");
    initVBO(vao1_got);
    first_size = gFaces.size()*3;

    gVertices.clear();
    gFaces.clear();
    gNormals.clear();


    ParseObj("bunny.obj");
    initVBO(vao2_got);
    second_size = gFaces.size()*3;

    gVertices.clear();
    gFaces.clear();
    gNormals.clear();


    ParseObj("cube.obj");
    initVBO(vao3_got);
    third_size = gFaces.size()*3;

    gVertices.clear();
    gFaces.clear();
    gNormals.clear();


    ParseObj("cube.obj");
    initVBO(vao4_got);
    fourth_size = gFaces.size()*3;

    gVertices.clear();
    gFaces.clear();
    gNormals.clear();

    ParseObj("cube.obj");
    initVBO(vao5_got);
    fifth_size = gFaces.size()*3;

    gVertices.clear();
    gFaces.clear();
    gNormals.clear();

    ParseObj("cube.obj");
    initVBO(vao6_got);
    sixth_size = gFaces.size()*3;

    gVertices.clear();
    gFaces.clear();
    gNormals.clear();

    ParseObj("cube.obj");
    initVBO(vao7_got);
    seventh_size = gFaces.size()*3;

    gVertices.clear();
    gFaces.clear();
    gNormals.clear();

    ParseObj("cube.obj");
    initVBO(vao8_got);
    eigth_size = gFaces.size()*3;

    gVertices.clear();
    gFaces.clear();
    gNormals.clear();

    ParseObj("cube.obj");
    initVBO(vao9_got);
    ninth_size = gFaces.size()*3;

    gVertices.clear();
    gFaces.clear();
    gNormals.clear();

    ParseObj("cube.obj");
    initVBO(vao10_got);
    tenth_size = gFaces.size()*3;

    gVertices.clear();
    gFaces.clear();
    gNormals.clear();

    ParseObj("cube.obj");
    initVBO(vao11_got);
    eleventh_size = gFaces.size()*3;

    glEnable(GL_DEPTH_TEST);
    initShaders();
    initTexture();
    glActiveTexture(GL_TEXTURE0);

}

void drawModel(int count)
{
    if(count == 0){
        glDepthMask(GL_FALSE);
        glBindVertexArray(vao0_got);
        glDrawElements(GL_TRIANGLES, zero_size, GL_UNSIGNED_INT, 0);
        glDepthMask(GL_TRUE);
    }else if(count == 1){
        glBindVertexArray(vao1_got);
        glDrawElements(GL_TRIANGLES, first_size+6, GL_UNSIGNED_INT, 0);

    }else if(count == 2){

        glBindVertexArray(vao2_got);
        glDrawElements(GL_TRIANGLES, second_size, GL_UNSIGNED_INT, 0);

    }else if(count == 3){

        glBindVertexArray(vao3_got);
        glDrawElements(GL_TRIANGLES, third_size, GL_UNSIGNED_INT, 0);

    }else if(count == 4){

        glBindVertexArray(vao4_got);
        glDrawElements(GL_TRIANGLES, fourth_size, GL_UNSIGNED_INT, 0);

    }else if(count == 5){

        glBindVertexArray(vao5_got);
        glDrawElements(GL_TRIANGLES, fifth_size, GL_UNSIGNED_INT, 0);

    }else if(count == 6){

        glBindVertexArray(vao6_got);
        glDrawElements(GL_TRIANGLES, sixth_size, GL_UNSIGNED_INT, 0);
    }else if(count == 7){

        glBindVertexArray(vao7_got);
        glDrawElements(GL_TRIANGLES, seventh_size, GL_UNSIGNED_INT, 0);

    }else if(count == 8){

        glBindVertexArray(vao8_got);
        glDrawElements(GL_TRIANGLES, eigth_size, GL_UNSIGNED_INT, 0);
    }else if(count == 9){

        glBindVertexArray(vao9_got);
        glDrawElements(GL_TRIANGLES, ninth_size, GL_UNSIGNED_INT, 0);
    }else if(count == 10){

        glBindVertexArray(vao10_got);
        glDrawElements(GL_TRIANGLES, tenth_size, GL_UNSIGNED_INT, 0);

    }else if(count == 11){

        glBindVertexArray(vao11_got);
        glDrawElements(GL_TRIANGLES, eleventh_size, GL_UNSIGNED_INT, 0);

    }

}
	bool hit = false;
    	
void display()
{

	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0f);
	glClearStencil(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    if(yel_poz1 == 0){
        yel1 = -6.0;
        red11 = 0.0;
        red12 = 6.0;
    }else if(yel_poz1 == 1){
        red11 = -6.0;
        yel1 =  0.0;
        red12 = 6.0;
    }else{
        red11 = -6.0;
        red12 = 0.0;
        yel1 =  6.0;
    }

    if(yel_poz2 == 0){
        yel2 = -6.0;
        red21 = 0.0;
        red22 = 6.0;
    }else if(yel_poz2 == 1){
        red21 = -6.0;
        yel2 =  0.0;
        red22 = 6.0;
    }else{
        red21 = -6.0;
        red22 = 0.0;
        yel2 =  6.0;
    }

    if(yel_poz3 == 0){
        yel3 = -6.0;
        red31 = 0.0;
        red32 = 6.0;
    }else if(yel_poz3 == 1){
        red31 = -6.0;
        yel3 =  0.0;
        red32 = 6.0;
    }else{
        red31 = -6.0;
        red32 = 0.0;
        yel3 =  6.0;
    }

    int active_side = 0;
    if(nearest_row == 1){
        nearest_row_max_z = block_z + 0.0;
        nearest_row_min_z = block_z - 1.0;
        active_side = yel_poz1;
    }else if(nearest_row == 2){
        nearest_row_max_z = block_z2 + 1.0;
        nearest_row_min_z = block_z2 - 1.0;
        active_side = yel_poz2;
    }else if(nearest_row == 3){
        nearest_row_max_z = block_z3 + 1.0;
        nearest_row_min_z = block_z3 - 1.0;
        active_side = yel_poz3;
    }


    if(((5.3 + bunny_z_min) < nearest_row_max_z) && (5.3 + bunny_z_max > nearest_row_max_z)){

        if(active_side == 0){
            if(((posx + bunny_x_min) < 0.70 && (posx + bunny_x_max) > -0.70) || (posx + bunny_x_max) > 5.30){
                dead = true;
            }else if((posx + bunny_x_min) < -5.30){
                inside = true;
                hit = true;
                spin = true;
            }
        }else if(active_side == 1){
            if((posx + bunny_x_min) < -5.30 || (posx + bunny_x_max) > 5.30){
                dead = true;
            }else if((posx + bunny_x_max) > -0.70 && (posx + bunny_x_min) < 0.70){
                inside = true;
                hit = true;
                spin = true;
            }
        }else if(active_side == 2){
            if((posx + bunny_x_min) < -5.30 || (posx + bunny_x_min) < 0.70 && (posx + bunny_x_max) > -0.70){
                dead = true;
            }else if((posx + bunny_x_max) > 5.30){
                inside = true;
                hit = true;
                spin = true;
            }

        }
    }else{
        if(inside){
            hit = true;	
            score += 1000;
            change_speed += 0.005;
            inside = false;
            spin = true;
        }
    }

    for(int i = 0;i< 12; i++){

        //plane transformations
        if(i == 0){
            modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.f, 0.f, -20.f));
            modelingMatrix = glm::rotate<float>(modelingMatrix, (-180. / 180.) * M_PI, glm::vec3(0.0, 1.0, 0.0));
            modelingMatrix = glm::scale(modelingMatrix, glm::vec3 (1000,1000,1000));
        }else if(i == 1){
            modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.f, 0.f, 0.f));
            modelingMatrix = glm::rotate<float>(modelingMatrix, (-90. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
            modelingMatrix = glm::scale(modelingMatrix, glm::vec3 (7,100,-100));
            //modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.f, 0.f, ));

        //bunny transformations
        }else if(i == 2){
            if(dead == true){
                modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(posx, 0.f, 6.f));
                modelingMatrix = glm::rotate<float>(modelingMatrix, (-90. / 180.) * M_PI, glm::vec3(0.0, 1.0, 0.0));
                modelingMatrix = glm::rotate<float>(modelingMatrix, (-90. / 180.) * M_PI, glm::vec3(1.0, 0.0, .0));
            }else{
                if(jump == true){
                    if (left){
                        if(posx >= -4.5 + bunny_x_min){
                            posx -= change_speed;
                        }
                    }else if(right){
                        if(posx <= 5 + bunny_x_max){
                            posx += change_speed;
                        }
                    }

                    modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(posx, jump_height, 6.f));

                    if(jump_height >= 1.5){
                        hit_top = true;
                    }
                    if(hit_top == false){
                        jump_height += change_speed;
                    }else{
                        jump_height -= change_speed;
                        if(jump_height <= 0.5){
                            hit_top = false;

                            jump_height = 0.5;
                        }
                    }
                    if(spin){
                        modelingMatrix = glm::rotate<float>(modelingMatrix, (-90. / 180.) * M_PI, glm::vec3(0.0, 1.0, 0.0));
                        modelingMatrix = glm::rotate<float>(modelingMatrix, (-spin_deg / 180.) * M_PI, glm::vec3(0.0, 1.0, 0.0));
                        spin_deg += change_speed*50;
                        if(spin_deg + change_speed*50 >= 350){
                            spin = false;
                            spin_deg = 0;
                        }else{
                            spin_deg += change_speed*50;
                        }
                    }else{
                         modelingMatrix = glm::rotate<float>(modelingMatrix, (-90. / 180.) * M_PI, glm::vec3(0.0, 1.0, 0.0));
                    }
                    
                }else{
                    modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(posx, 0.f, 3.f));
                }

             }

        //first row set
        }else if(i == 3){

            modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(red11, 1.5, block_z));
            modelingMatrix = glm::rotate<float>(modelingMatrix, (-90. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
            modelingMatrix = glm::scale(modelingMatrix, glm::vec3 (0.85,1,2));
        }else if(i == 4){

            modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(red12, 1.5, block_z));
            modelingMatrix = glm::rotate<float>(modelingMatrix, (-90. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
            modelingMatrix = glm::scale(modelingMatrix, glm::vec3 (0.85,1,2));
        }else if(i == 5){

            if(hit && nearest_row == 1){
                modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(yel1, 1.5, initial_block_z3));
                modelingMatrix = glm::rotate<float>(modelingMatrix, (-90. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
                modelingMatrix = glm::scale(modelingMatrix, glm::vec3 (0.85,1,2));
            }else{
                modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(yel1, 1.5, block_z));
                modelingMatrix = glm::rotate<float>(modelingMatrix, (-90. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
                modelingMatrix = glm::scale(modelingMatrix, glm::vec3 (0.85,1,2));
            }

            //second row set
        }else if(i == 6){

            modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(red21, 1.5, block_z2));
            modelingMatrix = glm::rotate<float>(modelingMatrix, (-90. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
            modelingMatrix = glm::scale(modelingMatrix, glm::vec3 (0.85,0.9,2));
        }else if(i == 7){

            modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(red22, 1.5, block_z2));
            modelingMatrix = glm::rotate<float>(modelingMatrix, (-90. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
            modelingMatrix = glm::scale(modelingMatrix, glm::vec3 (0.85,0.9,2));
        }else if(i == 8){

	    if(hit && nearest_row == 2){
	        modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(yel2, 1.5, initial_block_z3));
            	modelingMatrix = glm::rotate<float>(modelingMatrix, (-90. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
            	modelingMatrix = glm::scale(modelingMatrix, glm::vec3 (0.85,0.9,2));
	    }else{
	    	modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(yel2, 1.5, block_z2));
            	modelingMatrix = glm::rotate<float>(modelingMatrix, (-90. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
            	modelingMatrix = glm::scale(modelingMatrix, glm::vec3 (0.85,0.9,2));
	    }


            //third row set
        }else if(i == 9){

            modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(red31, 1.5, block_z3));
            modelingMatrix = glm::rotate<float>(modelingMatrix, (-90. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
            modelingMatrix = glm::scale(modelingMatrix, glm::vec3 (0.85,0.9,2));
        }else if(i == 10){

            modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(red32, 1.5, block_z3));
            modelingMatrix = glm::rotate<float>(modelingMatrix, (-90. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
            modelingMatrix = glm::scale(modelingMatrix, glm::vec3 (0.85,0.9,2));
        }else if(i == 11){

            if(hit && nearest_row == 3){
	        modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(yel3, 1.5, initial_block_z3));
            	modelingMatrix = glm::rotate<float>(modelingMatrix, (-90. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
            	modelingMatrix = glm::scale(modelingMatrix, glm::vec3 (0.85,0.9,2));
	    }else{
	    	modelingMatrix = glm::translate(glm::mat4(1.0), glm::vec3(yel3, 1.5, block_z3));
            	modelingMatrix = glm::rotate<float>(modelingMatrix, (-90. / 180.) * M_PI, glm::vec3(1.0, 0.0, 0.0));
            	modelingMatrix = glm::scale(modelingMatrix, glm::vec3 (0.85,0.9,2));
	    }

        }


        activeProgramIndex = i;
        glUseProgram(gProgram[activeProgramIndex]);
        GLint speedLocation = glGetUniformLocation(gProgram[activeProgramIndex], "speed");

        GLint texLocation = glGetUniformLocation(gProgram[activeProgramIndex], "o_texture");
        glUniform1i(texLocation, 0);
        if(!dead){
            glUniform1f(speedLocation, speed+= (change_speed/5.8));
        }
        GLint coord_loc = glGetUniformLocation(gProgram[activeProgramIndex], "texCoord");
        glUniform2f(coord_loc, 0.0, 1.0);
        glUniformMatrix4fv(projectionMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(projectionMatrix));
        glUniformMatrix4fv(viewingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(viewingMatrix));
        glUniformMatrix4fv(modelingMatrixLoc[activeProgramIndex], 1, GL_FALSE, glm::value_ptr(modelingMatrix));
        glUniform3fv(eyePosLoc[activeProgramIndex], 1, glm::value_ptr(eyePos));

        drawModel(i);

    }

    if(!dead){
        block_z += change_speed*2;
        block_z2 += change_speed*2;
        block_z3 += change_speed*2;
    }

    if(block_z >= 15){
        block_z = initial_block_z3;
        yel_poz1 = rand() % 3;
        nearest_row = 2;
        hit = false;
    }

    if(block_z2 >= 15){
        block_z2 = initial_block_z3;
        yel_poz2 = rand() % 3;
        nearest_row = 3;
        hit = false;
    }

    if(block_z3 >= 15){
        block_z3 = initial_block_z3;
        yel_poz3 = rand() % 3;
        nearest_row = 1;
        hit = false;
    }
}

void onWindowEvent(GLFWwindow* window, int w, int h)
{
	w = w < 1 ? 1 : w;
	h = h < 1 ? 1 : h;

	gWidth = w;
	gHeight = h;

	glViewport(0, 0, w, h);

	// Use perspective projection
	float fovyRad = (float)(90.0 / 180.0) * M_PI;
	projectionMatrix = glm::perspective(fovyRad, w / (float)h, 1.0f, 100.0f);

	// Assume default camera position and orientation (camera is at
	// (0, 0, 0) with looking at -z direction and its up vector pointing
	// at +y direction)
	//
	//viewingMatrix = glm::mat4(1);
	viewingMatrix = glm::lookAt(glm::vec3(0, 4.8, 13), glm::vec3(0, 2, -10) + glm::vec3(0, 0, -0.8), glm::vec3(0, 1, 0));

}

void onKeyEvent(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_R && action == GLFW_PRESS)
	{
		dead = false;
		hit = false;
        yel_poz1 = rand() % 3;
        yel_poz2 = rand() % 3;
        yel_poz3 = rand() % 3;
        if(nearest_row == 1){
            block_z = -45.0;
            block_z2 = -90.0;
            block_z3 = -135.0;
        }else if(nearest_row == 2){
            block_z2 = -45.0;
            block_z3 = -90.0;
            block_z = -135.0;
        }else if(nearest_row == 3){
            block_z3 = -45.0;
            block_z = -90.0;
            block_z2 = -135.0;
        }
        score = 0;
        speed = 0.0;

        change_speed = 0.05;

	}else if(key == GLFW_KEY_R && action == GLFW_REPEAT){
	dead = false;
	hit = false;
        yel_poz1 = rand() % 3;
        yel_poz2 = rand() % 3;
        yel_poz3 = rand() % 3;
        if(nearest_row == 1){
            block_z = -45.0;
            block_z2 = -90.0;
            block_z3 = -135.0;
        }else if(nearest_row == 2){
            block_z2 = -45.0;
            block_z3 = -90.0;
            block_z = -135.0;
        }else if(nearest_row == 3){
            block_z3 = -45.0;
            block_z = -90.0;
            block_z2 = -135.0;
        }
        score = 0;
        speed = 0.0;

        change_speed = 0.05;
	
	}else if (key == GLFW_KEY_A && action == GLFW_PRESS)
	{
        left = true;
        right = false;

	}
    else if(key == GLFW_KEY_A && action == GLFW_RELEASE){
        left = false;
    }
	else if (key == GLFW_KEY_D && action == GLFW_PRESS)
	{
        right = true;
        left = false;

	}
    else if (key == GLFW_KEY_D && action == GLFW_RELEASE)
    {
        right = false;

    }

}

void mainLoop(GLFWwindow* window)
{
	while (!glfwWindowShouldClose(window))
	{
	char window_title[20]; 
    	sprintf(window_title, "Score: %d", score);
    	
	glfwSetWindowTitle(window, window_title);
        glDepthMask(true);
        if(!dead){
            score += 1;
            change_speed += 0.0001;
        }


		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}

static void onError(const int code, const char *description) {
    fprintf(stderr, "GLFW Error: %s (0x%X)\n", description, code);
}

int main()
{
    glfwSetErrorCallback(onError);
	if (!glfwInit())
	{
		exit(EXIT_FAILURE);
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);


    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int width = 1000;
    int height = 800;

	GLFWwindow* window = glfwCreateWindow(width, height, "hw3", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);

	glfwSwapInterval(1);
    glfwWindowHint(GLFW_SAMPLES, 16);

    glfwSetKeyCallback(window, onKeyEvent);
    glfwSetWindowSizeCallback(window, onWindowEvent);

    if (const GLenum result = glewInit(); result != GLEW_OK)
	{
		std::cout << "GLEW Error: " << result << std::endl;
		return EXIT_FAILURE;
	}

    auto versionInfo = std::string((const char*)glGetString(GL_VERSION));
    auto rendererInfo = std::string((const char*)glGetString(GL_RENDERER));

    std::cout << "OpenGL: " << versionInfo << std::endl;
    std::cout << "Renderer: " << rendererInfo << std::endl;

	init();

    onWindowEvent(window, width, height);
	mainLoop(window);

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
