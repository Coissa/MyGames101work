//
// Created by Göksu Güvendiren on 2019-05-14.
//

#include "Scene.hpp"


void Scene::buildBVH() {
    printf(" - Generating BVH...\n\n");
    this->bvh = new BVHAccel(objects, 1, BVHAccel::SplitMethod::NAIVE);
}

Intersection Scene::intersect(const Ray &ray) const
{
    return this->bvh->Intersect(ray);
}

void Scene::sampleLight(Intersection &pos, float &pdf) const
{
    float emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
        }
    }
    float p = get_random_float() * emit_area_sum;
    emit_area_sum = 0;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        if (objects[k]->hasEmit()){
            emit_area_sum += objects[k]->getArea();
            if (p <= emit_area_sum){
                objects[k]->Sample(pos, pdf);
                break;
            }
        }
    }
}

bool Scene::trace(
        const Ray &ray,
        const std::vector<Object*> &objects,
        float &tNear, uint32_t &index, Object **hitObject)
{
    *hitObject = nullptr;
    for (uint32_t k = 0; k < objects.size(); ++k) {
        float tNearK = kInfinity;
        uint32_t indexK;
        Vector2f uvK;
        if (objects[k]->intersect(ray, tNearK, indexK) && tNearK < tNear) {
            *hitObject = objects[k];
            tNear = tNearK;
            index = indexK;
        }
    }


    return (*hitObject != nullptr);
}

// Implementation of Path Tracing
Vector3f Scene::castRay(const Ray &ray, int depth) const
{   
    auto p = intersect(ray);
    auto o = ray.origin;
    auto dir = ray.direction;
    if (!p.happened) {
        return Vector3f(0,0,0);
    }
    if (p.m->hasEmission()) {
        if (depth == 0) {
            return p.m->getEmission();
        }
        else return Vector3f(0);
    }
    Intersection light_inter;
    float light_pdf = 0.0f;
    
    sampleLight(light_inter, light_pdf);
    Vector3f x = light_inter.coords;
    Vector3f NN = light_inter.normal.normalized();
    Vector3f emit = light_inter.emit;
    auto ws = x - p.coords;
    float dist_sq = pow(ws.x, 2) + pow(ws.y, 2) + pow(ws.z, 2);
    float dist = std::sqrt(dist_sq); 
    Vector3f wi = ws.normalized();
    Vector3f wo = -ray.direction;
    auto fr = p.m->eval(wi, wo, p.normal.normalized());
    Ray shadow_ray(p.coords + EPSILON * wi, wi);
    Intersection block_inter = intersect(shadow_ray);
    Vector3f Ldir(0.0f);
    if (block_inter.happened && (block_inter.distance - dist > -0.01)) {
        if (light_pdf > EPSILON) {
            Ldir = (emit * fr) * std::max(0.0f, dotProduct(wi, p.normal.normalized())) * std::max(0.0f, dotProduct(-wi, NN)) / dist_sq / light_pdf;
        }
    }
    Vector3f Lindir(0.0f);
    if (get_random_float() < RussianRoulette) {
        wi = p.m->sample(wo, p.normal.normalized()).normalized();
        Ray r(p.coords + EPSILON * p.normal.normalized(), wi);
        fr = p.m->eval(wi, wo, p.normal.normalized());
        Lindir = castRay(r,depth + 1) * fr * std::max(0.0f, dotProduct(wi, p.normal.normalized())) / p.m->pdf(wo, wi, p.normal.normalized()) / RussianRoulette;
    }
    return Lindir + Ldir;
    // TO DO Implement Path Tracing Algorithm here
}