#include <algorithm>
#include <cassert>
#include "BVH.hpp"

BVHAccel::BVHAccel(std::vector<Object*> p, int maxPrimsInNode,
                   SplitMethod splitMethod)
    : maxPrimsInNode(std::min(255, maxPrimsInNode)), splitMethod(splitMethod),
      primitives(std::move(p))
{
    time_t start, stop;
    time(&start);
    if (primitives.empty())
        return;

    root = recursiveBuild(primitives);

    time(&stop);
    double diff = difftime(stop, start);
    int hrs = (int)diff / 3600;
    int mins = ((int)diff / 60) - (hrs * 60);
    int secs = (int)diff - (hrs * 3600) - (mins * 60);

    printf(
        "\rBVH Generation complete: \nTime Taken: %i hrs, %i mins, %i secs\n\n",
        hrs, mins, secs);
}

BVHBuildNode* BVHAccel::recursiveBuild(std::vector<Object*> objects)
{
    BVHBuildNode* node = new BVHBuildNode();

    // Compute bounds of all primitives in BVH node
    Bounds3 bounds;
    for (int i = 0; i < objects.size(); ++i)
        bounds = Union(bounds, objects[i]->getBounds());
    if (objects.size() == 1) {
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        return node;
    }
    else if (objects.size() == 2) {
        node->left = recursiveBuild(std::vector{ objects[0] });
        node->right = recursiveBuild(std::vector{ objects[1] });

        node->bounds = Union(node->left->bounds, node->right->bounds);
        return node;
    }
    else {
        Bounds3 centroidBounds;
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds =
            Union(centroidBounds, objects[i]->getBounds().Centroid());
        int dim = centroidBounds.maxExtent();
        switch (dim) {
        case 0:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().x <
                    f2->getBounds().Centroid().x;
                });
            break;
        case 1:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().y <
                    f2->getBounds().Centroid().y;
                });
            break;
        case 2:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().z <
                    f2->getBounds().Centroid().z;
                });
            break;
        }
        auto S0 = 1 / bounds.SurfaceArea();
        float mint = std::numeric_limits<double>::max();
        int mini = -1;
        std::vector<Bounds3> leftBounds(objects.size());
        std::vector<Bounds3> rightBounds(objects.size());
        Bounds3 tempLeft;
        for (int i = 0; i < objects.size(); ++i) {
            tempLeft = Union(tempLeft, objects[i]->getBounds());
            leftBounds[i] = tempLeft;
        }
        // 从右到左扫描，计算后 i 个物体的包围盒
        Bounds3 tempRight;
        for (int i = objects.size() - 1; i >= 0; --i) {
            tempRight = Union(tempRight, objects[i]->getBounds());
            rightBounds[i] = tempRight;
        }
        for (int i = 1; i < objects.size(); ++i) {
            auto S1 = leftBounds[i - 1].SurfaceArea();
            auto S2 = rightBounds[i].SurfaceArea();
            auto times = S1 * S0 * i + S2 * S0 * (objects.size() - i);
            if (times <= mint) {
                mini = i;
                mint = times;
            }
        }
        /*for (int i = 1; i < objects.size(); i++) {
            auto beginning = objects.begin();
            auto middling = objects.begin() + i;
            auto ending = objects.end();
            Bounds3 bounds1, bounds2;
            for (int j = 0; j < i ; ++j)
                bounds1 = Union(bounds1, objects[j]->getBounds());
            for (int j = i; j < objects.size(); ++j)
                bounds2 = Union(bounds2, objects[j]->getBounds());
            auto S1 = bounds1.SurfaceArea();
            auto S2 = bounds2.SurfaceArea();
            auto times = S1 * S0 * i + S2 * S0 * (objects.size() - i);
            if (times <= mint) {
                mint = times;
                mini = i;
            }
        }*/
        if (mini == -1) mini = objects.size() / 2;
        auto beginning = objects.begin();
        auto middling = objects.begin() + mini;
        auto ending = objects.end();

        auto leftshapes = std::vector<Object*>(beginning, middling);
        auto rightshapes = std::vector<Object*>(middling, ending);

        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursiveBuild(leftshapes);
        node->right = recursiveBuild(rightshapes);

        node->bounds = Union(node->left->bounds, node->right->bounds);
    }

    return node;
}

/*
BVHBuildNode* BVHAccel::recursiveBuild(std::vector<Object*> objects)
{
    BVHBuildNode* node = new BVHBuildNode();

    // Compute bounds of all primitives in BVH node
    Bounds3 bounds;
    for (int i = 0; i < objects.size(); ++i)
        bounds = Union(bounds, objects[i]->getBounds());
    if (objects.size() == 1) {
        // Create leaf _BVHBuildNode_
        node->bounds = objects[0]->getBounds();
        node->object = objects[0];
        node->left = nullptr;
        node->right = nullptr;
        return node;
    }
    else if (objects.size() == 2) {
        node->left = recursiveBuild(std::vector{objects[0]});
        node->right = recursiveBuild(std::vector{objects[1]});

        node->bounds = Union(node->left->bounds, node->right->bounds);
        return node;
    }
    else {
        Bounds3 centroidBounds;
        for (int i = 0; i < objects.size(); ++i)
            centroidBounds =
                Union(centroidBounds, objects[i]->getBounds().Centroid());
        int dim = centroidBounds.maxExtent();
        switch (dim) {
        case 0:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().x <
                       f2->getBounds().Centroid().x;
            });
            break;
        case 1:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().y <
                       f2->getBounds().Centroid().y;
            });
            break;
        case 2:
            std::sort(objects.begin(), objects.end(), [](auto f1, auto f2) {
                return f1->getBounds().Centroid().z <
                       f2->getBounds().Centroid().z;
            });
            break;
        }

        auto beginning = objects.begin();
        auto middling = objects.begin() + (objects.size() / 2);
        auto ending = objects.end();

        auto leftshapes = std::vector<Object*>(beginning, middling);
        auto rightshapes = std::vector<Object*>(middling, ending);

        assert(objects.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursiveBuild(leftshapes);
        node->right = recursiveBuild(rightshapes);

        node->bounds = Union(node->left->bounds, node->right->bounds);
    }

    return node;
}
*/
Intersection BVHAccel::Intersect(const Ray& ray) const
{
    Intersection isect;
    if (!root)
        return isect;
    isect = BVHAccel::getIntersection(root, ray);
    return isect;
}

Intersection BVHAccel::getIntersection(BVHBuildNode* node, const Ray& ray) const
{

    Intersection inter;
    if (node == nullptr) {
        return inter;
    }
    std::array<int, 3> dirIsNeg = {
    int(ray.direction.x > 0),
    int(ray.direction.y > 0),
    int(ray.direction.z > 0)
    };
    // TODO Traverse the BVH to find intersection
    if (node->bounds.IntersectP(ray, ray.direction_inv, dirIsNeg) == false) {
        return inter;
    }
    if (node->left == nullptr && node->right == nullptr) {
        return node->object->getIntersection(ray);
    }
    Intersection a;
    Intersection b;
    if (node->left) {
        a = getIntersection(node->left, ray);
    }
    if (node->right) {
        b = getIntersection(node->right, ray);
    }
    if (a.happened == true && b.happened == true) {
        if (a.distance > b.distance) {
            return b;
        }
        else {
            return a;
        }
    }
    if (a.happened == true) {
        return a;
    }
    else {
        return b;
    }
}