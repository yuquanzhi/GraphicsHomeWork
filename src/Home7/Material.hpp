//
// Created by LEI XU on 5/16/19.
//

#ifndef RAYTRACING_MATERIAL_H
#define RAYTRACING_MATERIAL_H

#include "Vector.hpp"
#include "glm/glm.hpp"
#include "global.hpp"

enum MaterialType { DIFFUSE, Microfacet };
using vec3 = glm::vec3;
using namespace glm ;


class Material{
private:

    // Compute reflection direction
    Vector3f reflect(const Vector3f &I, const Vector3f &N) const
    {
        return I - 2 * dotProduct(I, N) * N;
    }

    // Compute refraction direction using Snell's law
    //
    // We need to handle with care the two possible situations:
    //
    //    - When the ray is inside the object
    //
    //    - When the ray is outside.
    //
    // If the ray is outside, you need to make cosi positive cosi = -N.I
    //
    // If the ray is inside, you need to invert the refractive indices and negate the normal N
    Vector3f refract(const Vector3f &I, const Vector3f &N, const float &ior) const
    {
        float cosi = clamp(-1, 1, dotProduct(I, N));
        float etai = 1, etat = ior;
        Vector3f n = N;
        if (cosi < 0) { cosi = -cosi; } else { std::swap(etai, etat); n= -N; }
        float eta = etai / etat;
        float k = 1 - eta * eta * (1 - cosi * cosi);
        return k < 0 ? 0 : eta * I + (eta * cosi - sqrtf(k)) * n;
    }

    // Compute Fresnel equation
    //
    // \param I is the incident view direction
    //
    // \param N is the normal at the intersection point
    //
    // \param ior is the material refractive index
    //
    // \param[out] kr is the amount of light reflected
    void fresnel(const Vector3f &I, const Vector3f &N, const float &ior, float &kr) const
    {
        float cosi = clamp(-1, 1, dotProduct(I, N));
        float etai = 1, etat = ior;
        if (cosi > 0) {  std::swap(etai, etat); }
        // Compute sini using Snell's law
        float sint = etai / etat * sqrtf(std::max(0.f, 1 - cosi * cosi));
        // Total internal reflection
        if (sint >= 1) {
            kr = 1;
        }
        else {
            float cost = sqrtf(std::max(0.f, 1 - sint * sint));
            cosi = fabsf(cosi);
            float Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
            float Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
            kr = (Rs * Rs + Rp * Rp) / 2;
        }
        // As a consequence of the conservation of energy, transmittance is given by:
        // kt = 1 - kr;
    }

    Vector3f toWorld(const Vector3f &a, const Vector3f &N){
        Vector3f B, C;
        if (std::fabs(N.x) > std::fabs(N.y)){
            float invLen = 1.0f / std::sqrt(N.x * N.x + N.z * N.z);
            C = Vector3f(N.z * invLen, 0.0f, -N.x *invLen);
        }
        else {
            float invLen = 1.0f / std::sqrt(N.y * N.y + N.z * N.z);
            C = Vector3f(0.0f, N.z * invLen, -N.y *invLen);
        }
        B = crossProduct(C, N);
        return a.x * B + a.y * C + a.z * N;
    }

	float DistributionGGX(vec3 N, vec3 H, float roughness) {
		float a = roughness * roughness;
		float a2 = a * a;
		float NdotH = max(dot(N, H), 0.0f);
		float NdotH2 = NdotH * NdotH;

		float denom = (NdotH2 * (a2 - 1.0) + 1.0);
		denom = M_PI * denom * denom;

		return a2 / denom;
	}

	float GeometrySchlickGGX(float NdotV, float roughness) {
		float r = (roughness + 1.0);
		float k = (r * r) / 8.0;

		float denom = NdotV * (1.0 - k) + k;
		return NdotV / denom;
	}

	float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
		float NdotV = max(dot(N, V), 0.0f);
		float NdotL = max(dot(N, L), 0.0f);
		float ggx1 = GeometrySchlickGGX(NdotV, roughness);
		float ggx2 = GeometrySchlickGGX(NdotL, roughness);

		return ggx1 * ggx2;
	}

	vec3 fresnelSchlick(float cosTheta, vec3 F0) {
		return F0 + (1.0f - F0) * (float)pow(1.0 - cosTheta, 5.0);
	}

	vec3 MicrofacetBRDF(vec3 N, vec3 V, vec3 L, vec3 albedo, float metallic, float roughness) {
		vec3 H = glm::normalize(V + L);

		// 计算各分量
		vec3 F0 = mix(vec3(0.04), albedo, metallic);
		vec3 F = fresnelSchlick(max(dot(H, V), 0.0f), F0);

		float NDF = DistributionGGX(N, H, roughness);
		float G = GeometrySmith(N, V, L, roughness);

		// 组合BRDF
		vec3 numerator = NDF * G * F;
		float denominator = 4.0 * max(dot(N, V), 0.0f) * max(dot(N, L), 0.0f);
		vec3 specular = numerator / max(denominator, 0.001f);

		// 漫反射部分
		vec3 kD = (vec3(1.0) - F) * (float)(1.0 - metallic);
		vec3 diffuse = kD * albedo / M_PI;

		return diffuse + specular;
	}


public:
    MaterialType m_type;
    //Vector3f m_color;
    Vector3f m_emission;
    float ior;
    Vector3f Kd, Ks;
    float specularExponent;
    //Texture tex;

    //额外添加的BRDF属性
	float metallic;
	float roughness;
    vec3 albedo;

    inline Material(MaterialType t=DIFFUSE, Vector3f e=Vector3f(0,0,0));
	inline Material(MaterialType t, float metallic, float roughness, vec3 albedo);
    inline MaterialType getType();
    //inline Vector3f getColor();
    inline Vector3f getColorAt(double u, double v);
    inline Vector3f getEmission();
    inline bool hasEmission();

    // sample a ray by Material properties
    inline Vector3f sample(const Vector3f &wi, const Vector3f &N);
    // given a ray, calculate the PdF of this ray
    inline float pdf(const Vector3f &wi, const Vector3f &wo, const Vector3f &N);
    // given a ray, calculate the contribution of this ray
    inline Vector3f eval(const Vector3f &wi, const Vector3f &wo, const Vector3f &N);

};

Material::Material(MaterialType t, Vector3f e){
    m_type = t;
    //m_color = c;
    m_emission = e;
}

Material::Material(MaterialType t, float metallic, float roughness, vec3 albedo):m_type(t), metallic(metallic), roughness(roughness), albedo(albedo){
	//m_type = t;
	//m_color = c;
	//m_emission = e;
}


MaterialType Material::getType(){return m_type;}
///Vector3f Material::getColor(){return m_color;}
Vector3f Material::getEmission() {return m_emission;}
bool Material::hasEmission() {
    if (m_emission.norm() > EPSILON) return true;
    else return false;
}

Vector3f Material::getColorAt(double u, double v) {
    return Vector3f();
}


Vector3f Material::sample(const Vector3f &wi, const Vector3f &N){
    switch(m_type){
        case DIFFUSE:
        {
            // uniform sample on the hemisphere
            float x_1 = get_random_float(), x_2 = get_random_float();
            float z = std::fabs(1.0f - 2.0f * x_1);
            float r = std::sqrt(1.0f - z * z), phi = 2 * M_PI * x_2;
            Vector3f localRay(r*std::cos(phi), r*std::sin(phi), z);
            return toWorld(localRay, N);
            
            break;
        }
        case Microfacet:
        {
			// uniform sample on the hemisphere
			float x_1 = get_random_float(), x_2 = get_random_float();
			float z = std::fabs(1.0f - 2.0f * x_1);
			float r = std::sqrt(1.0f - z * z), phi = 2 * M_PI * x_2;
			Vector3f localRay(r * std::cos(phi), r * std::sin(phi), z);
			return toWorld(localRay, N);

			break;

        }
		


    }
}

float Material::pdf(const Vector3f &wi, const Vector3f &wo, const Vector3f &N){
    switch(m_type){
        case DIFFUSE:
        {
            // uniform sample probability 1 / (2 * PI)
            if (dotProduct(wo, N) > 0.0f)
                return 0.5f / M_PI;
            else
                return 0.0f;
            break;
        }
		case Microfacet:
		{
			// uniform sample probability 1 / (2 * PI)
				if (dotProduct(wo, N) > 0.0f)
					return 0.5f / M_PI;
				else
					return 0.0f;
			break;
		}
    }
}

Vector3f Material::eval(const Vector3f &wi, const Vector3f &wo, const Vector3f &N){
    switch(m_type){
        case DIFFUSE:
        {
            // calculate the contribution of diffuse   model
            float cosalpha = dotProduct(N, wo);
            if (cosalpha > 0.0f) {
                Vector3f diffuse = Kd / M_PI;
                return diffuse;
            }
            else
                return Vector3f(0.0f);
            break;
        }
		case Microfacet:
		{
			// calculate the contribution of microfacet model
            Vector3f tempWi = -wi;

			Vector3f H = normalize(tempWi + wo);
			float cosThetaV = dotProduct(N, wo);
			float cosThetaL = dotProduct(N, tempWi);
			if (cosThetaV > 0.0f && cosThetaL > 0.0f) {
                vec3 temp=  MicrofacetBRDF(vec3(N.x,N.y,N.z),vec3(wo.x, wo.y, wo.z) ,vec3(tempWi.x, tempWi.y, tempWi.z) , albedo, metallic, roughness);
                Vector3f specular = Vector3f(temp.x, temp.y, temp.z) ;
				return specular;
			}
			else
				return Vector3f(0.0f);
		}
    }
}

#endif //RAYTRACING_MATERIAL_H
