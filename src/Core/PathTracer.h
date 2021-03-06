#pragma once

#include "Core/Camera.h"
#include "Core/Renderer.h"
#include "Materials/MaterialManager.h"
#include "Primitives/SceneObjectList.h"
#include "Utils/ctpl.h"

namespace core
{

class PathTracer : public Renderer
{
  public:
	PathTracer() = default;

	~PathTracer() override;

	explicit PathTracer(prims::WorldScene *scene, int width, int height, Camera *camera, Surface *skyBox = nullptr);

	inline void SetMode(Mode mode) override
	{
		m_Mode = mode;
		Reset();
	}

	void Reset() override;

	int GetSamples() const override;

	void Render(Surface *output) override;

	glm::vec3 Trace(Ray &r, uint &depth, float refractionIndex, RandomGenerator &rng);

	bool TraceLightRay(Ray &r, prims::SceneObject *light) const; // returns whether light is obstructed

	glm::vec3 SampleNEE(Ray &r, RandomGenerator &rng) const;

	glm::vec3 SampleIS(Ray &r, RandomGenerator &rng) const;

	glm::vec3 SampleReference(Ray &r, RandomGenerator &rng) const;

	glm::vec3 SampleNEE_IS(Ray &r, RandomGenerator &rng) const;

	glm::vec3 SampleNEE_MIS(Ray &r, RandomGenerator &rng) const;

	glm::vec3 SampleSkyBox(const glm::vec3 &dir) const;

	glm::vec3 SampleReferenceMicrofacet(Ray &r, RandomGenerator &rng) const;

	glm::vec3 SampleNEEMicrofacet(Ray &r, RandomGenerator &rng) const;

	glm::vec3 Refract(const bool &flipNormal, const Material &mat, const glm::vec3 &normal, const glm::vec3 &p,
					  const float &t, Ray &r, RandomGenerator &rng) const;

	float GetEnergy() const;

	float TotalEnergy() const;

	inline void SwitchSkybox() override
	{
		this->m_SkyboxEnabled = (this->m_SkyBox != nullptr && !this->m_SkyboxEnabled);
	}

	prims::SceneObject *RandomPointOnLight(float &NEEpdf, RandomGenerator &rng) const;

	inline bool RussianRoulette(glm::vec3 &throughput, const uint depth, RandomGenerator &rng) const
	{
		if (depth > 3)
		{
			const float probability = std::max(EPSILON, std::max(throughput.x, std::max(throughput.y, throughput.z)));
			if (rng.Rand(1.0f) > probability || probability <= 0.f)
				return true;
			throughput /= probability;
		}
		return false;
	}

	void Resize(gl::Texture *newOutput) override;

	void SetMode(std::string mode) override
	{
		if (mode == "NEE")
			SetMode(NEE);
		else if (mode == "IS")
			SetMode(IS);
		else if (mode == "NEE_IS")
			SetMode(NEE_IS);
		else if (mode == "NEE_MIS")
			SetMode(NEE_MIS);
		else if (mode == "Reference MF")
			SetMode(ReferenceMicrofacet);
		else if (mode == "Reference")
			SetMode(Reference);
	}

  private:
	prims::WorldScene *m_Scene;
	glm::vec3 *m_Pixels;
	float *m_Energy;
	int m_Width, m_Height, m_Samples;
	std::vector<float> m_LightLotteryTickets;
	float m_LightArea;
	unsigned int m_LightCount;
	Surface *m_SkyBox = nullptr;
	bool m_SkyboxEnabled = false;
	Camera *m_Camera;
	const MaterialManager *m_Materials;

	int m_Tiles;

	ctpl::ThreadPool *tPool = nullptr;
	std::vector<std::future<void>> tResults{};
	std::vector<RandomGenerator *> m_Rngs{};
};
} // namespace core
