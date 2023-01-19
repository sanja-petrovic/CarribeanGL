#version 330 core

struct PositionalLight {
	vec3 Position;
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float Kc;
	float Kl;
	float Kq;
};

struct DirectionalLight {
	vec3 Position;
	vec3 Direction;
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	float InnerCutOff;
	float OuterCutOff;
	float Kc;
	float Kl;
	float Kq;
};

struct Material {
	// NOTE(Jovan): Diffuse is used as ambient as well since the light source
	// defines the ambient colour
	sampler2D Kd;
	sampler2D Ks;
	float Shininess;
};

uniform PositionalLight uPointLight;
uniform PositionalLight uPointLight2;
uniform PositionalLight uPointLight3;
uniform DirectionalLight uSpotlight;
uniform DirectionalLight uDirLight;
uniform Material uMaterial;
uniform vec3 uViewPos;

in vec2 UV;
in vec3 vWorldSpaceFragment;
in vec3 vWorldSpaceNormal;

out vec4 FragColor;

void main() {
	vec3 ViewDirection = normalize(uViewPos - vWorldSpaceFragment);
	// NOTE(Jovan): Directional light
	vec3 DirLightVector = normalize(-uDirLight.Direction);
	float DirDiffuse = max(dot(vWorldSpaceNormal, DirLightVector), 0.0f);
	vec3 DirReflectDirection = reflect(-DirLightVector, vWorldSpaceNormal);
	// NOTE(Jovan): 32 is the specular shininess factor. Hardcoded for now
	float DirSpecular = pow(max(dot(ViewDirection, DirReflectDirection), 0.0f), uMaterial.Shininess);

	vec3 DirAmbientColor = uDirLight.Ka * vec3(texture(uMaterial.Kd, UV));
	vec3 DirDiffuseColor = uDirLight.Kd * DirDiffuse * vec3(texture(uMaterial.Kd, UV));
	vec3 DirSpecularColor = uDirLight.Ks * DirSpecular * vec3(texture(uMaterial.Ks, UV));
	vec3 DirColor = DirAmbientColor + DirDiffuseColor + DirSpecularColor;

	// NOTE(Jovan): Point light
	vec3 PtLightVector = normalize(uPointLight.Position - vWorldSpaceFragment);
	float PtDiffuse = max(dot(vWorldSpaceNormal, PtLightVector), 0.0f);
	vec3 PtReflectDirection = reflect(-PtLightVector, vWorldSpaceNormal);
	float PtSpecular = pow(max(dot(ViewDirection, PtReflectDirection), 0.0f), uMaterial.Shininess);

	vec3 PtAmbientColor = uPointLight.Ka * vec3(texture(uMaterial.Kd, UV));
	vec3 PtDiffuseColor = PtDiffuse * uPointLight.Kd * vec3(texture(uMaterial.Kd, UV));
	vec3 PtSpecularColor = PtSpecular * uPointLight.Ks * vec3(texture(uMaterial.Ks, UV));

	float PtLightDistance = length(uPointLight.Position - vWorldSpaceFragment);
	float PtAttenuation = 1.0f / (uPointLight.Kc + uPointLight.Kl * PtLightDistance + uPointLight.Kq * (PtLightDistance * PtLightDistance));
	vec3 PtColor = PtAttenuation * (PtAmbientColor + PtDiffuseColor + PtSpecularColor);

	// Point light 2
	vec3 PtLightVector2 = normalize(uPointLight2.Position - vWorldSpaceFragment);
	float PtDiffuse2 = max(dot(vWorldSpaceNormal, PtLightVector2), 0.0f);
	vec3 PtReflectDirection2 = reflect(-PtLightVector2, vWorldSpaceNormal);
	float PtSpecular2 = pow(max(dot(ViewDirection, PtReflectDirection2), 0.0f), uMaterial.Shininess);

	vec3 PtAmbientColor2 = uPointLight2.Ka * vec3(texture(uMaterial.Kd, UV));
	vec3 PtDiffuseColor2 = PtDiffuse2 * uPointLight2.Kd * vec3(texture(uMaterial.Kd, UV));
	vec3 PtSpecularColor2 = PtSpecular2 * uPointLight2.Ks * vec3(texture(uMaterial.Ks, UV));

	float PtLightDistance2 = length(uPointLight2.Position - vWorldSpaceFragment);
	float PtAttenuation2 = 1.0f / (uPointLight2.Kc + uPointLight2.Kl * PtLightDistance2 + uPointLight2.Kq * (PtLightDistance2 * PtLightDistance2));
	vec3 PtColor2 = PtAttenuation2 * (PtAmbientColor2 + PtDiffuseColor2 + PtSpecularColor2);

	// Point light 3
	vec3 PtLightVector3 = normalize(uPointLight3.Position - vWorldSpaceFragment);
	float PtDiffuse3 = max(dot(vWorldSpaceNormal, PtLightVector3), 0.0f);
	vec3 PtReflectDirection3 = reflect(-PtLightVector3, vWorldSpaceNormal);
	float PtSpecular3 = pow(max(dot(ViewDirection, PtReflectDirection3), 0.0f), uMaterial.Shininess);

	vec3 PtAmbientColor3 = uPointLight3.Ka * vec3(texture(uMaterial.Kd, UV));
	vec3 PtDiffuseColor3 = PtDiffuse3 * uPointLight3.Kd * vec3(texture(uMaterial.Kd, UV));
	vec3 PtSpecularColor3 = PtSpecular3 * uPointLight3.Ks * vec3(texture(uMaterial.Ks, UV));

	float PtLightDistance3 = length(uPointLight3.Position - vWorldSpaceFragment);
	float PtAttenuation3 = 1.0f / (uPointLight3.Kc + uPointLight3.Kl * PtLightDistance3 + uPointLight3.Kq * (PtLightDistance3 * PtLightDistance3));
	vec3 PtColor3 = PtAttenuation3 * (PtAmbientColor3 + PtDiffuseColor3 + PtSpecularColor3);

	// NOTE(Jovan): Spotlight
	vec3 SpotlightVector = normalize(uSpotlight.Position - vWorldSpaceFragment);

	float SpotDiffuse = max(dot(vWorldSpaceNormal, SpotlightVector), 0.0f);
	vec3 SpotReflectDirection = reflect(-SpotlightVector, vWorldSpaceNormal);
	float SpotSpecular = pow(max(dot(ViewDirection, SpotReflectDirection), 0.0f), uMaterial.Shininess);

	vec3 SpotAmbientColor = uSpotlight.Ka * vec3(texture(uMaterial.Kd, UV));
	vec3 SpotDiffuseColor = SpotDiffuse * uSpotlight.Kd * vec3(texture(uMaterial.Kd, UV));
	vec3 SpotSpecularColor = SpotSpecular * uSpotlight.Ks * vec3(texture(uMaterial.Ks, UV));

	float SpotlightDistance = length(uSpotlight.Position - vWorldSpaceFragment);
	float SpotAttenuation = 2.5f / (uSpotlight.Kc + uSpotlight.Kl * SpotlightDistance + uSpotlight.Kq * (SpotlightDistance * SpotlightDistance));

	float Theta = dot(SpotlightVector, normalize(-uSpotlight.Direction));
	float Epsilon = uSpotlight.InnerCutOff - uSpotlight.OuterCutOff;
	float SpotIntensity = clamp((Theta - uSpotlight.OuterCutOff) / Epsilon, 0.0f, 1.0f);
	vec3 SpotColor = SpotIntensity * SpotAttenuation * (SpotAmbientColor + SpotDiffuseColor + SpotSpecularColor);
	
	vec3 FinalColor = DirColor + PtColor + PtColor2 + PtColor3 + SpotColor;
	FragColor = vec4(FinalColor, 1.0f);
}