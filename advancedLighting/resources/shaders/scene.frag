#version 330

//Interface Block
in VS_OUT
{
	in vec3 FragPos;
	in vec2 TextCoord;
	in vec3 FragNormal;
	
	//Tangent space position conversions.
	vec3 TangentLightPos;
    vec3 TangentViewPos;
    vec3 TangentFragPos;
}fs_in;

//Light Uniform Data
struct LightAttr
{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
uniform LightAttr light;

//User Input Variables
//uniform bool bParallaxMapping;
//uniform float heightScale;
uniform bool bNormalMapping;

//Model Textures
uniform sampler2D texture_diffuse0;
uniform sampler2D texture_specular0;
uniform sampler2D texture_normal0;
//uniform sampler2D texture_height0;

//Final Pixel Colour Output Location
out vec4 color;

//Function to offset the texture fragment with parallax.
//vec2 parallaxMapping(vec2 textCoord,vec3 viewDir)
//{
	//float height = texture(texture_height, textCoord).r;
	//vec2  offset = viewDir.xy / viewDir.z * (height * heightScale);
	//return textCoord - offset;
//}

void main()
{   
	//vec3 viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
	//vec2 textCoord = fs_in.TextCoord;

	//Parallax map texture fragment if toggled to.
	//if(bParallaxMapping)
	//{
		//textCoord = parallaxMapping(fs_in.TextCoord, viewDir);
		
		//Discard if out of texture co-ordinate range.
		//if(textCoord.x < 0.0 || textCoord.y < 0.0 || textCoord.x > 1.0 || textCoord.y > 1.0)
		//{	
		//discard;
		//}
	//}
   // vec3 objectColor = texture(texture_diffuse0,textCoord).rgb;

	//Ambient Light Colour Contribution
	vec3 ambient = light.ambient * vec3(texture(texture_diffuse0, fs_in.TextCoord));

	//Normal Mapping
	vec3 lightDir = normalize(fs_in.TangentLightPos - fs_in.TangentFragPos);
	vec3 normal = texture(fs_in.FragNormal);
	
	//Only normal map model when user interaction has toggled for it.
	if(bNormalMapping)  
	{
		normal = texture(texture_normal0, fs_in.TextCoord).rgb;
		normal = normalize(normal * 2.0 - 1.0); //Constrain within range.
	}
	
	//Diffusion Light Colour Contribution
	float diffFactor = max(dot(lightDir, normal), 0.0);
	vec3 diffuse = diffFactor * light.diffuse * vec3(texture(texture_diffuse0, fs_in.TextCoord));

	//Specular Light Colour Contribution
	vec3    viewDir = normalize(fs_in.TangentViewPos - fs_in.TangentFragPos);
	float	specFactor = 0.0;
	vec3	halfDir = normalize(lightDir + viewDir);
	specFactor = pow(max(dot(halfDir, normal), 0.0), 32.0); 
	vec3	specular = specFactor * light.specular * vec3(texture(texture_specular0, fs_in.TextCoord));
	
	//Resultant Final Colour from Combined Contributions
	vec3	result = (ambient + diffuse + specular ); //* objectColor;
	color	= vec4(result , 1.0f);
}