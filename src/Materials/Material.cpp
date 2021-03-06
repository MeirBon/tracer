#include "Material.h"
#include "MaterialManager.h"

Material::Material(float diffuse, glm::vec3 colorDiffuse, float shininess, float refractionIndex, float transparency,
				   glm::vec3 absorption)
{
	this->diffuse = diffuse;
	this->colorDiffuse = colorDiffuse;
	this->shininess = glm::min(shininess, 1.0f);
	this->refractionIndex = refractionIndex;
	this->transparency = transparency;
	this->absorption = absorption;
}

Material::Material(float diffuse, glm::vec3 colorDiffuse, float shininess, float refractionIndex, float transparency)
{
	this->diffuse = diffuse;
	this->colorDiffuse = colorDiffuse;
	this->shininess = glm::min(shininess, 1.0f);
	this->refractionIndex = refractionIndex;
	this->transparency = transparency;
	this->absorption = glm::vec3(0.f);
}

Material::Material(float diffuse, glm::vec3 colorDiffuse, float shininess, float refractionIndex, float transparency,
				   glm::vec3 absorption, unsigned int texIdx)
{
	this->diffuse = diffuse;
	this->colorDiffuse = colorDiffuse;
	this->shininess = glm::min(shininess, 1.0f);
	this->refractionIndex = refractionIndex;
	this->transparency = transparency;
	this->absorption = absorption;
	this->textureIdx = texIdx;
}

Material::Material(glm::vec3 emittance, float intensity)
{
	this->emission = emittance;
	this->intensity = intensity;
	this->MakeLight();
}

Material::Material(float diffuse, glm::vec3 albedo, float refractionIndex, float transparency, glm::vec3 absorption)
{
	this->diffuse = diffuse;
	this->shininess = 8;
	this->albedo = albedo;
	this->refractionIndex = refractionIndex;
	this->transparency = transparency;
	this->absorption = absorption;
}

Material::Material(float diffuse, glm::vec3 colorDiffuse, float shininess, unsigned int textureIdx)
{
	this->diffuse = diffuse;
	this->colorDiffuse = colorDiffuse;
	this->shininess = glm::min(shininess, 1.0f);
	this->refractionIndex = 1.0f;
	this->absorption = glm::vec3(0.f);
	this->textureIdx = textureIdx;
}

glm::vec3 Material::GetDiffuseColor(const prims::SceneObject *obj, const glm::vec3 &hitPoint) const
{
	if (this->textureIdx > -1)
		return MaterialManager::GetInstance()->GetTexture(textureIdx).GetColorAt(obj->GetTexCoords(hitPoint));
	else
		return this->colorDiffuse;
}

glm::vec3 Material::GetAlbedoColor(const prims::SceneObject *obj, const glm::vec3 &hitPoint) const
{
	if (this->textureIdx > -1)
		return MaterialManager::GetInstance()->GetTexture(textureIdx).GetColorAt(obj->GetTexCoords(hitPoint));
	else
		return this->colorDiffuse;
}