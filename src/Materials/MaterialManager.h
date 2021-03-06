#pragma once

#include <vector>

#include "Core/Surface.h"

#include "Materials/Material.h"
#include "Materials/Microfacet.h"

class MaterialManager
{
  public:
	static MaterialManager *GetInstance();

	Material &GetMaterial(size_t index);
	Microfacet &GetMicrofacet(size_t index);
	core::Surface &GetTexture(size_t index);

	std::vector<Material> &GetMaterials() { return m_Materials; }

	std::vector<Microfacet> &GetMicrofacets() { return m_Microfacets; }

	std::vector<core::Surface *> &GetTextures() { return m_Textures; }

	const Material &GetMaterial(size_t index) const;
	const Microfacet &GetMicrofacet(size_t index) const;
	const core::Surface &GetTexture(size_t index) const;

	size_t AddMaterial(Material mat);
	size_t AddMicrofacet(Microfacet mat);
	size_t AddTexture(const char *path);

	static void Delete();

  private:
	MaterialManager() = default;
	~MaterialManager() = default;

	std::vector<Material> m_Materials;
	std::vector<Microfacet> m_Microfacets;

	std::vector<core::Surface *> m_Textures;
};
