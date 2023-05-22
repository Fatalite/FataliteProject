// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h" 
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "SwordActor.h"
#include "Async/ParallelFor.h"
#include "FMyAsyncSimulationTask.h"
#include "RealtimeMeshSimple.h"
#include <vector>
#include <array>
#include <limits>
#include <algorithm>
#include <iostream>
#include <set>
#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif
#include <map>
#include <stack>
#include <numeric>
#include <DynamicMesh/DynamicMesh3.h>
#include "CylinderActor.generated.h"
using namespace std;


template<typename T>
T sorted(const T& v) {
    auto sv = v;
    sort(sv.begin(), sv.end());
    return sv;
}

class UnionFind {
    int* id, cnt, * sz;
public:
    // Create an empty union find data structure with N isolated sets.
    UnionFind(int N) {
        cnt = N;
        id = new int[N];
        sz = new int[N];
        for (int i = 0; i < N; i++) {
            id[i] = i;
            sz[i] = 1;
        }
    }
    ~UnionFind() {
        delete[] id;
        delete[] sz;
    }
    // Return the id of component corresponding to object p.
    int find(int p) {
        int root = p;
        while (root != id[root])
            root = id[root];
        while (p != root) {
            int newp = id[p];
            id[p] = root;
            p = newp;
        }
        return root;
    }
    // Replace sets containing x and y with their union.
    void merge(int x, int y) {
        int i = find(x);
        int j = find(y);
        if (i == j) return;

        // make smaller root point to larger one
        if (sz[i] < sz[j]) {
            id[i] = j;
            sz[j] += sz[i];
        }
        else {
            id[j] = i;
            sz[i] += sz[j];
        }
        cnt--;
    }
    // Are objects x and y in the same set?
    bool connected(int x, int y) {
        return find(x) == find(y);
    }
    // Return the number of disjoint sets.
    int count() {
        return cnt;
    }
};

template<typename T, int d>
array<T, 4> toI4(const array<T, d>& Id, T fill = -1) {
    array<T, 4> a;
    a.fill(fill);
    for (int i = 0; i < min(d, 4); ++i) {
        a[i] = Id[i];
    }
    return a;
}
template<typename T, int d>
array<T, d> add(const array<T, d>& a1, const array<T, d>& a2) {
    array<T, d> s;
    for (int j = 0; j < d; ++j) {
        s[j] = a1[j] + a2[j];
    }
    return s;
}

template<typename T, int d>
array<T, d> substract(const array<T, d>& a1, const array<T, d>& a2) {
    array<T, d> s;
    for (int j = 0; j < d; ++j) {
        s[j] = a1[j] - a2[j];
    }
    return s;
}

template<typename T, int d>
array<T, d> divide(const array<T, d>& a, T t) {
    array<T, d> s;
    for (int j = 0; j < d; ++j) {
        s[j] = a[j] / t;
    }
    return s;
}

template<typename T, int d, int n>
array<T, d> center(const array<array<T, d>, n>& nodes, const array<T, n> weights) {
    array<T, d> c;
    for (size_t i = 0; i < d; ++i) {
        c[i] = 0;
        for (size_t j = 0; j < n; ++j) {
            c[i] += nodes[j][i] * weights[j];
        }
    }
    return c;
}

template<typename T, int d>
array<T, d> elementCenter(const vector<array<T, d>>& nodes, const array<int, 4>& element, const array<T, 4>& weights) {
    array<T, d> c;
    c.fill(0);
    for (size_t i = 0; i < 4; ++i) {
        if (element[i] >= 0) {
            const auto& node = nodes[element[i]];
            auto w = weights[i];
            for (int j = 0; j < d; ++j) {
                c[j] += (node[j] * w);
            }
        }
        else {
            break;
        }
    }
    return c;
}

template<typename T, int d>
T dot(const array<T, d>& a1, const array<T, d>& a2) {
    T s = 0;
    for (int j = 0; j < d; ++j) {
        s += a1[j] * a2[j];
    }
    return s;
}

template<typename T>
T cross(const array<T, 2>& a1, const array<T, 2>& a2) {
    return a1[0] * a2[1] - a1[1] * a2[0];
}

template<typename T>
array<T, 3> cross(const array<T, 3>& a1, const array<T, 3>& a2) {
    array<T, 3> s;
    for (int j = 0; j < 3; ++j) {
        s[j] = a1[(j + 1) % 3] * a2[(j + 2) % 3] - a2[(j + 1) % 3] * a1[(j + 2) % 3];
    }
    return s;
}

template<typename T>
T volume(const array<T, 3>& node1, const array<T, 3>& node2, const array<T, 3>& node3, const array<T, 3>& node4) {
    return dot<T, 3>(cross<T>(substract<T, 3>(node2, node1), substract<T, 3>(node3, node1)), substract<T, 3>(node4, node1));
}

template<typename T, int d>
T norm(const array<T, d>& a1) {
    return sqrt(dot(a1, a1));
}

template<typename T, int d>
T pointEdgeWeight(const array<T, d>& e1, const array<T, d>& e2, const array<T, d>& p) {
    array<T, d> v1 = substract<T, d>(p, e1);
    array<T, d> v2 = substract<T, d>(e2, e1);
    return dot<T, d>(v1, v2) / dot<T, d>(v2, v2);
}

template<typename T, int d>
bool pointOnEdge(const array<T, d>& e1, const array<T, d>& e2, const array<T, d>& p, T& w) {
    array<T, d> v1 = substract<T, d>(p, e1);
    array<T, d> v2 = substract<T, d>(e1, e2);
    if (cross<T>(v1, v2) == 0) {
        w = pointEdgeWeight<T, d>(e1, e2, p);
        return true;
    }
    return false;
}

template<typename T>
bool edgeEdgeIntersect(const array<T, 2>& p1, const array<T, 2>& p2, const array<T, 2>& q1, const array<T, 2>& q2, T& w) {
    array<T, 2> e1 = substract<T, 2>(p2, p1);
    T a1 = cross<T>(substract<T, 2>(q1, p1), e1);
    T a2 = cross<T>(substract<T, 2>(q2, p1), e1);
    if ((a1 < 0 && a2 > 0) || (a1 > 0 && a2 < 0)) {
        array<T, 2> e2 = substract<T, 2>(q2, q1);
        T a3 = cross<T>(substract<T, 2>(p1, q1), e2);
        T a4 = cross<T>(substract<T, 2>(p2, q1), e2);
        if ((a3 < 0 && a4 > 0) || (a3 > 0 && a4 < 0)) {
            w = a3 / (a3 - a4);
            return true;
        }
    }
    return false;
}

template<typename T>
bool pointInTriangle(const array<array<T, 2>, 3>& triangle, const array<T, 2>& point, array<T, 3>& w) {
    array<T, 3> areas;
    for (int i = 0; i < 3; ++i) {
        areas[(i + 2) % 3] = cross<T>(substract<T, 2>(point, triangle[i]), substract<T, 2>(triangle[(i + 1) % 3], triangle[i]));
    }
    if ((areas[0] < 0 && areas[1] < 0 && areas[2] < 0) || (areas[0] > 0 && areas[1] > 0 && areas[2] > 0)) {
        w = divide<T, 3>(areas, areas[0] + areas[1] + areas[2]);
        return true;
    }
    return false;
}

template<typename T, int d, int d2>
array<T, d> elementCenter(const vector<array<T, d>>& vertices, const array<int, d2>& element) {
    array<T, d> center;
    for (auto i : element) {
        for (int j = 0; j < d; ++j) {
            center[j] += vertices[i][j];
        }
    }
    for (int i = 0; i < d; ++i) {
        center[i] /= (T)element.size();
    }
    return center;
}

template<typename T, int d>
struct Box {
    array<T, d> lowerLeft_, upperRight_;

    Box() {}

    Box(const Box& b) {
        for (int i = 0; i < d; ++i) {
            lowerLeft_[i] = b.lowerLeft_[i];
            upperRight_[i] = b.upperRight_[i];
        }
    }

    Box(const Box& b1, const Box& b2) {
        for (int i = 0; i < d; ++i) {
            lowerLeft_[i] = min(b1.lowerLeft_[i], b2.lowerLeft_[i]);
            upperRight_[i] = max(b1.upperRight_[i], b2.upperRight_[i]);
        }
    }

    bool intersects(const Box& b) {
        for (int i = 0; i < d; ++i) {
            if (lowerLeft_[i] > b.upperRight_[i] || upperRight_[i] < b.lowerLeft_[i]) {
                return false;
            }
        }
        return true;
    }
};

template<typename T, int d, int d1>
Box<T, d> buildBox(const vector<array<T, d>>& vertices, const array<int, d1>& element) {
    Box<T, d> b;
    //    print<int,d1>(element);
    for (int i = 0; i < d; ++i) {
        b.lowerLeft_[i] = 1E+37;
        b.upperRight_[i] = numeric_limits<T>::lowest();
    }
    //    print<T,d>(b.lowerLeft_);
    //    print<T,d>(b.upperRight_);
    for (size_t i = 0; i < d1; ++i) {
        //        print<T,d>(vertices[element[i]]);
        for (int j = 0; j < d; ++j) {
            b.lowerLeft_[j] = min(b.lowerLeft_[j], vertices[element[i]][j]);
            b.upperRight_[j] = max(b.upperRight_[j], vertices[element[i]][j]);
        }
    }
    //    print<T,d>(b.lowerLeft_);
    //    print<T,d>(b.upperRight_);
    return b;
}

template<typename T, int d>
struct BoxNode {
    int n_;
    BoxNode* left_, * right_;
    Box<T, d> box_;

    BoxNode(int n, const vector<Box<T, d>>& boxes) : n_(n), left_(nullptr), right_(nullptr), box_(boxes[n]) {}

    BoxNode(BoxNode* left, BoxNode* right) : n_(-1), left_(left), right_(right), box_(left->box_, right->box_) {}

    ~BoxNode() {
        if (left_ != nullptr) {
            delete left_;
        }
        if (right_ != nullptr) {
            delete right_;
        }
    }
};

template<typename T, int d>
BoxNode<T, d>* buildBoxHierarchy(const vector<Box<T, d>>& boxes, const vector<array<T, d>>& centers, vector<size_t>& elementIndexes, int begin, int end, int level) {
    BoxNode<T, d>* root = nullptr;
    if (elementIndexes.size() == 0) {
        return nullptr;
    }
    if (begin == end) {
        root = new BoxNode<T, d>(elementIndexes[begin], boxes);
    }
    else {
        nth_element(elementIndexes.begin() + begin, elementIndexes.begin() + (begin + end) / 2, elementIndexes.begin() + end, [&](std::size_t i, std::size_t j) { return centers[i][level % d] < centers[j][level % d]; });
        BoxNode<T, d>* left = buildBoxHierarchy<T, d>(boxes, centers, elementIndexes, begin, (begin + end) / 2, ++level);
        BoxNode<T, d>* right = buildBoxHierarchy<T, d>(boxes, centers, elementIndexes, (begin + end) / 2 + 1, end, level);
        root = new BoxNode<T, d>(left, right);
    }
    return root;
}

template<typename T, int d>
class BoxHierarchy {
    int n_; //number of boxes
    BoxNode<T, d>* root_;
public:
    BoxHierarchy(const vector<Box<T, d>>& boxes, const vector<array<T, d>>& centers) : n_(centers.size()) {
        if (boxes.size()) {
            vector<size_t> elementIndexes(boxes.size());
            iota(elementIndexes.begin(), elementIndexes.end(), 0);
            root_ = buildBoxHierarchy<T, d>(boxes, centers, elementIndexes, 0, boxes.size() - 1, 0);
        }
    }

    void intersect(const BoxHierarchy<T, d>& bh, vector<vector<int>>& intersectingElements) const {
        intersectingElements.clear();
        intersectingElements.resize(n_);
        if (!root_ || !bh.root_) {
            return;
        }
        stack<pair<BoxNode<T, d>*, BoxNode<T, d>*>> s;
        s.push(pair<BoxNode<T, d>*, BoxNode<T, d>*>(root_, bh.root_));
        while (s.size()) {
            pair<BoxNode<T, d>*, BoxNode<T, d>*> top = s.top();
            s.pop();
            if (top.first->box_.intersects(top.second->box_)) {
                if (top.first->n_ != -1 && top.second->n_ != -1) {
                    intersectingElements[top.first->n_].push_back(top.second->n_);
                }
                else if (top.second->n_ != -1) {
                    s.push(pair<BoxNode<T, d>*, BoxNode<T, d>*>(top.first->left_, top.second));
                    s.push(pair<BoxNode<T, d>*, BoxNode<T, d>*>(top.first->right_, top.second));
                }
                else if (top.first->n_ != -1) {
                    s.push(pair<BoxNode<T, d>*, BoxNode<T, d>*>(top.first, top.second->left_));
                    s.push(pair<BoxNode<T, d>*, BoxNode<T, d>*>(top.first, top.second->right_));
                }
                else {
                    s.push(pair<BoxNode<T, d>*, BoxNode<T, d>*>(top.first->left_, top.second->left_));
                    s.push(pair<BoxNode<T, d>*, BoxNode<T, d>*>(top.first->left_, top.second->right_));
                    s.push(pair<BoxNode<T, d>*, BoxNode<T, d>*>(top.first->right_, top.second->left_));
                    s.push(pair<BoxNode<T, d>*, BoxNode<T, d>*>(top.first->right_, top.second->right_));
                }
            }
        }
    }

    ~BoxHierarchy() {
        delete root_;
    }
};

template<typename T, int d1, int d2>
BoxHierarchy<T, d1> buildBoxHierarchy(const vector<array<T, d1>>& nodes, const vector<array<int, d2>>& elements) {
    vector<Box<T, d1>> boxes;
    vector<array<T, d1>> centers;
    for (const auto& e : elements) {
        boxes.push_back(buildBox<T, d1, d2>(nodes, e));
        centers.push_back(elementCenter<T, d1, d2>(nodes, e));
    }
    return BoxHierarchy<T, d1>(boxes, centers);
}

template<typename T>
class TriMesh {
    typedef std::array<int, 2> I2;
    typedef std::array<int, 3> I3;
    typedef std::array<int, 4> I4;
    typedef std::array<T, 3> TV;

public:
    vector<TV> nodes_;
    vector<I3> mesh_;

    void clear() {
        nodes_.clear();
        mesh_.clear();
    }
};

const array<array<int, 3>, 4> FaceIndexes = {
    array<int, 3>{0,1,2},
    array<int, 3>{0,2,3},
    array<int, 3>{1,2,3},
    array<int, 3>{0,3,1}
};

inline array<array<int, 3>, 4> tetFaces(const array<int, 4>& tet) {
    array<array<int, 3>, 4> faces;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 3; ++j) {
            faces[i][j] = tet[FaceIndexes[i][j]];
        }
    }
    return faces;
}

inline array<int, 3> tetFace(const array<int, 4>& tet, int i) {
    array<int, 3> face;
    for (int j = 0; j < 3; ++j) {
        face[j] = tet[FaceIndexes[i][j]];
    }
    return face;
}

const array<array<int, 2>, 6> EdgeIndexes = { array<int,2>{0,1}, array<int,2>{0,2}, array<int,2>{0,3}, array<int,2>{1,2}, array<int,2>{1,3}, array<int,2>{2,3} };

inline array<array<int, 2>, 6> tetEdges(const array<int, 4>& tet) {
    array<array<int, 2>, 6> faces;
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 2; ++j) {
            faces[i][j] = tet[EdgeIndexes[i][j]];
        }
    }
    return faces;
}

inline array<array<int, 2>, 3> faceEdges(const array<int, 3>& face) {
    array<array<int, 2>, 3> edges;
    for (int i = 0; i < 3; ++i) {
        edges[i] = array<int, 2>{face[i], face[(i + 1) % 3]};
    }
    return edges;
}

template<typename T, int d, int d1>
inline array<array<T, d>, d1> elementNodes(const vector<array<T, d>>& nodes, const array<int, d1>& element) {
    array<array<T, d>, d1> ps;
    for (int i = 0; i < d1; ++i) {
        ps[i] = nodes[element[i]];
    }
    return ps;
}
template<typename T>
class TetMesh {
    typedef array<int, 2> I2;
    typedef array<int, 3> I3;
    typedef array<int, 4> I4;
    typedef array<T, 3> TV;

public:
    vector<TV> nodes_;
    vector<I4> mesh_;
    vector<I3> surfaceMesh_;
    vector<int> connectedComponents_; //connected component id of each element

    TetMesh() {}

    TetMesh(vector<TV>&& nodes, vector<I4>&& mesh) : nodes_(nodes), mesh_(mesh) {
        initializeSurfaceMesh();
        computeConnectedComponents();
    }
    void initializeSurfaceMesh() {
        surfaceMesh_.clear();
        map<I3, I3> surfaceElements; //sorted to unsorted elements
        for (const auto& tet : mesh_) {
            for (const auto& fi : FaceIndexes) {
                auto face = I3{ tet[fi[0]], tet[fi[1]], tet[fi[2]] };
                auto sortedFace = face;
                sort(sortedFace.begin(), sortedFace.end());
                if (surfaceElements.count(sortedFace)) {
                    surfaceElements.erase(sortedFace);
                }
                else {
                    surfaceElements[sortedFace] = face;
                }
            }
        }
        for (const auto& e : surfaceElements) {
            surfaceMesh_.push_back(e.second);
        }
    }

    void computeConnectedComponents() {
        connectedComponents_.resize(mesh_.size());
        UnionFind nodeClasses(nodes_.size());
        for (int i = 0; i < mesh_.size(); ++i) {
            for (int j = 0; j < 3; ++j) {
                nodeClasses.merge(mesh_[i][j], mesh_[i][j + 1]);
            }
        }
        map<int, int> nodeClassToCC;
        int c = 1;
        for (int i = 0; i < mesh_.size(); ++i) {
            if (!nodeClassToCC.count(nodeClasses.find(mesh_[i][0]))) {
                connectedComponents_[i] = c;
                nodeClassToCC[nodeClasses.find(mesh_[i][0])] = c;
                ++c;
            }
            else {
                connectedComponents_[i] = nodeClassToCC[nodeClasses.find(mesh_[i][0])];
            }
        }UE_LOG(LogTemp, Log, TEXT("found %d conneted components"),c-1);
       
    }
};


template<typename T>
class Cutter3D {
    typedef std::array<int, 1> I1;
    typedef std::array<int, 2> I2;
    typedef std::array<int, 3> I3;
    typedef std::array<int, 4> I4;
    typedef std::array<int, 5> I5;
    typedef std::array<T, 2> T2;
    typedef std::array<T, 3> T3;
    typedef std::array<T, 4> T4;
    typedef map<I4, T4> Intersections;
    typedef map<I4, vector<int>> TetBoundary2TetIds;
    //Cutted Element
    struct CutElement {
        int parentElementIndex;
        array<bool, 4> subElements; // in the same order as the tet nodes

        CutElement(int i, bool fill = true) : parentElementIndex(i) {
            subElements.fill(fill);
        }

        int numPieces() const {
            return (int)subElements[0] + (int)subElements[1] + (int)subElements[2] + (int)subElements[3];
        }
    };
    //triangle-triangle Intersection
    static bool computeIntersection(const array<T3, 2>& nodes1, const array<T3, 3>& nodes2, array<T, 2>& w1, array<T, 3>& w2) {
        T v1 = volume<T>(nodes1[0], nodes2[0], nodes2[1], nodes2[2]);
        T v2 = volume<T>(nodes1[1], nodes2[0], nodes2[1], nodes2[2]);
        T v3 = volume<T>(nodes1[0], nodes1[1], nodes2[0], nodes2[1]);
        T v4 = volume<T>(nodes1[0], nodes1[1], nodes2[1], nodes2[2]);
        T v5 = volume<T>(nodes1[0], nodes1[1], nodes2[2], nodes2[0]);
        if (v1 * v2 < 0 && (v3 > 0) == (v4 > 0) && (v4 > 0) == (v5 > 0)) {
            w1[0] = fabs(v2) / (fabs(v1) + fabs(v2));
            w1[1] = 1 - w1[0];
            T v = fabs(v3) + fabs(v4) + fabs(v5);
            w2[0] = fabs(v4) / v;
            w2[1] = fabs(v5) / v;
            w2[2] = 1 - w2[0] - w2[1];
            return true;
        }
        else {
            return false;
        }
    }

    static bool computeIntersection(const array<T3, 2>& nodes1, const array<T3, 3>& nodes2, array<T, 2>& w) {
        array<T, 3> w1;
        return computeIntersection(nodes1, nodes2, w, w1);
    }

    static bool computeIntersection(const array<T3, 3>& nodes1, const array<T3, 2>& nodes2, array<T, 3>& w) {
        array<T, 2> w1;
        return computeIntersection(nodes2, nodes1, w1, w);
    }

    static bool computeIntersection(const array<T3, 4>& nodes1, const array<T3, 1>& nodes2, array<T, 4>& w) {
        T v1 = volume<T>(nodes1[0], nodes1[1], nodes1[2], nodes2[0]);
        T v2 = volume<T>(nodes1[0], nodes1[2], nodes1[3], nodes2[0]);
        T v3 = volume<T>(nodes1[0], nodes1[3], nodes1[1], nodes2[0]);
        T v4 = volume<T>(nodes2[0], nodes1[1], nodes1[2], nodes1[3]);
        if (v1 == 0 || v2 == 0 || v3 == 0 || v4 == 0) {
        }
        if ((v1 > 0) == (v2 > 0) && (v2 > 0) == (v3 > 0) && (v3 > 0) == (v4 > 0)) {
            T v = fabs(v1) + fabs(v2) + fabs(v3) + fabs(v4);
            w[0] = fabs(v4) / v;
            w[1] = fabs(v2) / v;
            w[2] = fabs(v3) / v;
            w[3] = 1 - w[0] - w[1] - w[2];
            return true;
        }
        else {
            return false;
        }
        return false;
    }

    template<int d1, int d2>
    static void computeIntersections(const vector<T3>& nodes1, const vector<T3>& nodes2, const vector<array<int, d1>>& e1, const vector<array<int, d2>>& e2, const BoxHierarchy<T, 3>& b1, const BoxHierarchy<T, 3>& b2, map<I4, T4>& intersections) {
        vector<vector<int>> intersectingBoxes; // intersecting boxes
        b1.intersect(b2, intersectingBoxes);
        for (size_t i = 0; i < intersectingBoxes.size(); ++i) {
            for (auto j : intersectingBoxes[i]) {
                auto tetNodes = elementNodes<T, 3, d1>(nodes1, e1[i]);
                auto triNodes = elementNodes<T, 3, d2>(nodes2, e2[j]);
                array<T, d1> w;
                if (computeIntersection(tetNodes, triNodes, w)) {
                    intersections[toI4<int, d1>(e1[i])] = toI4<T, d1>(w, 0);
                }
            }
        }
    }

    static Intersections computeIntersections(const TetMesh<T>& tetMesh, const TriMesh<T>& triMesh, TetBoundary2TetIds& tetBoundary2TetIds) {
        map<I4, T4> intersections;
        // build box hierarchies for tetMesh
        set<I3> tetMeshFaces;
        set<I2> tetMeshEdges;
        for (int i = 0; i < tetMesh.mesh_.size(); ++i) {
            auto tet = tetMesh.mesh_[i];
            sort(tet.begin(), tet.end());
            //Tetrahedron mesh boundary to Tetrahedron Meshes?
            //It means just unique index?
            tetBoundary2TetIds[tet].push_back(i);
            auto faces = tetFaces(tet);
            for (auto& face : faces) {
                sort(face.begin(), face.end());
                tetBoundary2TetIds[toI4<int, 3>(face)].push_back(i);
                tetMeshFaces.insert(face);
            }
            auto edges = tetEdges(tet);
            for (auto& edge : edges) {
                sort(edge.begin(), edge.end());
                tetBoundary2TetIds[toI4<int, 2>(edge)].push_back(i);
                tetMeshEdges.insert(edge);
            }
        }
        vector<I1> tetMeshNodeVec;
        for (int i = 0; i < tetMesh.nodes_.size(); ++i) {
            tetMeshNodeVec.push_back(I1{ i });
        }
        vector<I3> tetMeshFaceVec(tetMeshFaces.begin(), tetMeshFaces.end());
        vector<I2> tetMeshEdgeVec(tetMeshEdges.begin(), tetMeshEdges.end());
        auto tetMeshHierarchy = buildBoxHierarchy<T, 3, 4>(tetMesh.nodes_, tetMesh.mesh_);
        auto tetMeshFaceHierarchy = buildBoxHierarchy<T, 3, 3>(tetMesh.nodes_, tetMeshFaceVec);
        auto tetMeshEdgeHierarchy = buildBoxHierarchy<T, 3, 2>(tetMesh.nodes_, tetMeshEdgeVec);
        auto tetMeshNodeHierarchy = buildBoxHierarchy<T, 3, 1>(tetMesh.nodes_, tetMeshNodeVec);

        // box hierarchy for triMesh
        set<I2> triMeshEdges;
        for (const auto& tri : triMesh.mesh_) {
            auto edges = faceEdges(tri);
            for (auto& edge : edges) {
                sort(edge.begin(), edge.end());
                triMeshEdges.insert(edge);
            }
        }
        vector<I2> triMeshEdgeVec(triMeshEdges.begin(), triMeshEdges.end());
        vector<I1> triMeshNodeVec;
        for (int i = 0; i < triMesh.nodes_.size(); ++i) {
            triMeshNodeVec.push_back(I1{ i });
        }
        auto triMeshHierarchy = buildBoxHierarchy<T, 3, 3>(triMesh.nodes_, triMesh.mesh_);
        auto triMeshEdgeHierarchy = buildBoxHierarchy<T, 3, 2>(triMesh.nodes_, triMeshEdgeVec);
        auto triMeshNodeHierarchy = buildBoxHierarchy<T, 3, 1>(triMesh.nodes_, triMeshNodeVec);

        // compute intersections
        // v-v
        // v-e
        // v-f
        // e-v
        // e-e
        // e-f
        computeIntersections<2, 3>(tetMesh.nodes_, triMesh.nodes_, tetMeshEdgeVec, triMesh.mesh_, tetMeshEdgeHierarchy, triMeshHierarchy, intersections);
        // f-v
        // f-e
        computeIntersections<3, 2>(tetMesh.nodes_, triMesh.nodes_, tetMeshFaceVec, triMeshEdgeVec, tetMeshFaceHierarchy, triMeshEdgeHierarchy, intersections);
        // t-v
        computeIntersections<4, 1>(tetMesh.nodes_, triMesh.nodes_, tetMesh.mesh_, triMeshNodeVec, tetMeshHierarchy, triMeshNodeHierarchy, intersections);

        return intersections;
    }

    static vector<CutElement> split(const TetMesh<T>& tetMesh, const Intersections& intersections, TetBoundary2TetIds& tetBoundary2TetIds, set<int>& cutTets) {
        cutTets.clear();
        //
        for (const auto& t : tetBoundary2TetIds) {
            if (intersections.count(t.first)) {
                for (auto i : t.second) {
                    cutTets.insert(i);
                }
            }
        }
        vector<CutElement> v;
        for (int i = 0; i < tetMesh.mesh_.size(); ++i) {
            //IF CutTet's one == TetMesh's one
            if (cutTets.count(i)) {
                array<bool, 4> added;
                added.fill(false);
                auto tet = tetMesh.mesh_[i];
                for (int j = 0; j < 4; ++j) {
                    if (!added[j]) {
                        // find all connected pieces
                        CutElement ce(i, false);
                        stack<int> s;
                        s.push(j);
                        //Graph finding
                        while (s.size()) {
                            auto top = s.top();
                            ce.subElements[top] = true;
                            added[top] = true;
                            s.pop();
                            // add all the connected pieces that are not added yet
                            for (int k = 0; k < 4; ++k) {
                                if (!added[k]) {
                                    if (!intersections.count(toI4<int, 2>(sorted(I2{ tet[top],tet[k] })))) {
                                        s.push(k);
                                    }
                                }
                            }
                        }
                        v.push_back(ce);
                    }
                }
            }
        }
        return v;
    }

    void static newTet(int parentId, const I4& tet, const TetMesh<T>& tetMesh, vector<T3>& newNodes, vector<I4>& newMesh, map<int, int>& nodeMapping, UnionFind& uf) {
        I4 newTet;
        for (int i = 0; i < 4; ++i) { // for each node
            int newId = uf.find(tet[i]);
            const auto& it = nodeMapping.find(newId);
            if (it != nodeMapping.end()) {
                newTet[i] = it->second;
            }
            else {
                newTet[i] = newNodes.size();
                nodeMapping[newId] = newNodes.size();
                newNodes.push_back(tetMesh.nodes_[tetMesh.mesh_[parentId][i]]);
            }
        }

        newMesh.push_back(newTet);
    }

    static void merge(const vector<CutElement>& cutElements, const TetMesh<T>& tetMesh, vector<T3>& newNodes, vector<I4>& newMesh, const Intersections& intersections) {
        newNodes.clear();
        newMesh.clear();
        UnionFind uf(tetMesh.nodes_.size() + 4 * cutElements.size());
        map<I5, int> faceNode2NewNode; // key = {face,materialNode,node}
        set<int> cutTets;
        int total = tetMesh.nodes_.size();
        for (const auto& ce : cutElements) { // need to do face-face merging even for tets that are touched by the cut but not split, so that if a neighbor splits they are all connected to it.
            cutTets.insert(ce.parentElementIndex);
            const auto& tet = tetMesh.mesh_[ce.parentElementIndex];
            for (int i = 0; i < 4; ++i) { // for each face
                auto face = tetFace(tet, i);
                sort(face.begin(), face.end());
                I5 key;
                for (int j = 0; j < 3; ++j) {
                    key[j] = face[j];
                }
                for (int j = 0; j < 3; ++j) { // for each node check for material
                    int fij = FaceIndexes[i][j];
                    if (ce.subElements[fij]) {
                        key[3] = tet[fij];
                        uf.merge(total + fij, key[3]);
                        for (int k = 0; k < 3; ++k) { // for each node, merge
                            int fik = FaceIndexes[i][k];
                            key[4] = tet[fik];
                            int newId = total + fik;
                            //print<int,5>(key);
                            const auto& it = faceNode2NewNode.find(key);
                            if (it != faceNode2NewNode.end()) {
                                //cout << "merging " << it->second << ", " << newId << endl;
                                uf.merge(it->second, newId);
                            }
                            else {
                                faceNode2NewNode[key] = newId;
                            }
                        }
                    }
                }
            }
            total += 4;
        }
        total = tetMesh.nodes_.size();
        map<int, int> nodeMapping;
        for (const auto& ce : cutElements) {
            newTet(ce.parentElementIndex, I4{ total, total + 1, total + 2, total + 3 }, tetMesh, newNodes, newMesh, nodeMapping, uf);
            total += 4;
        }
        for (int i = 0; i < tetMesh.mesh_.size(); ++i) {
            if (!cutTets.count(i)) {
                newTet(i, tetMesh.mesh_[i], tetMesh, newNodes, newMesh, nodeMapping, uf);
            }
        }

    }

    static TetMesh<T> subdivide(const vector<CutElement>& cutElements, const TetMesh<T>& tetMesh, vector<T3>& newNodes, vector<I4>& newMesh, Intersections& intersections) {
        // add a new node inside the tet, connect with cuts on each face to subdivide the tet
        map<I4, int> newNodeMapping;
        for (int i = 0; i < cutElements.size(); ++i) {
            const auto& ce = cutElements[i];
            const auto& originalTet = tetMesh.mesh_[ce.parentElementIndex];
            const auto sortedOriginalTet = sorted(originalTet);
            const auto& tet = newMesh[i];
            // get all edge cuts and add them as new nodes
            const auto originalEdges = tetEdges(originalTet);
            const auto edges = tetEdges(tet);
            int cutEdges = 0;
            T4 averageEdgeWeight{ 0,0,0,0 };
            map<int, T> originalNodeId2Weight;
            for (int k = 0; k < originalEdges.size(); ++k) {
                auto sortedOriginalEdge = toI4<int, 2>(sorted(originalEdges[k]));
                auto sortedEdge = toI4<int, 2>(sorted(edges[k]));
                const auto& it = intersections.find(sortedOriginalEdge);
                if (it != intersections.end()) {
                    ++cutEdges;
                    for (int j = 0; j < 2; ++j) {
                        originalNodeId2Weight[sortedOriginalEdge[j]] += it->second[j];
                    }
                    const auto& idIt = newNodeMapping.find(sortedEdge);
                    if (idIt == newNodeMapping.end()) {
                        newNodeMapping[sortedEdge] = newNodes.size();
                        newNodes.push_back(elementCenter<T, 3>(tetMesh.nodes_, sortedOriginalEdge, it->second));
                    }
                }
            }
            for (int j = 0; j < 4; ++j) {
                averageEdgeWeight[j] = originalNodeId2Weight[sortedOriginalTet[j]];
            }
            // face cuts
            const auto originalFaces = tetFaces(originalTet);
            const auto faces = tetFaces(tet);
            for (int k = 0; k < faces.size(); ++k) {
                auto sortedOriginalFace = toI4<int, 3>(sorted(originalFaces[k]));
                auto sortedFace = toI4<int, 3>(sorted(faces[k]));
                const auto& it = intersections.find(sortedOriginalFace);
                if (it != intersections.end()) { // face center already computed
                    const auto& idIt = newNodeMapping.find(sortedFace);
                    if (idIt == newNodeMapping.end()) {
                        newNodeMapping[sortedFace] = newNodes.size();
                        newNodes.push_back(elementCenter<T, 3>(tetMesh.nodes_, sortedOriginalFace, it->second));
                    }
                }
                else { // use average of edge cuts if not
                    int numEdges = 0;
                    T4 faceWeights{ 0,0,0,0 };
                    map<int, T> node2weight;
                    for (int j = 0; j < 3; ++j) {
                        auto sortedOriginalEdge = toI4<int, 2>(sorted(array<int, 2>{sortedOriginalFace[j], sortedOriginalFace[(j + 1) % 3]}));
                        const auto& edgeIt = intersections.find(sortedOriginalEdge);
                        if (edgeIt != intersections.end()) {
                            ++numEdges;
                            for (int e = 0; e < 2; ++e) {
                                node2weight[sortedOriginalEdge[e]] += edgeIt->second[e];
                            }
                        }
                    }
                    if (numEdges > 1) { // otherwise don't add new face center
                        newNodeMapping[sortedFace] = newNodes.size();
                        for (int j = 0; j < 3; ++j) {
                            faceWeights[j] = node2weight[sortedOriginalFace[j]] / numEdges;
                        }
                        newNodes.push_back(elementCenter<T, 3>(tetMesh.nodes_, sortedOriginalFace, faceWeights));
                        intersections[sortedOriginalFace] = faceWeights;
                    }
                }
            }
            int tetCenterId = newNodes.size();
            const auto& tetCenterIt = intersections.find(sortedOriginalTet);
            if (tetCenterIt != intersections.end()) {
                newNodes.push_back(elementCenter<T, 3>(tetMesh.nodes_, sortedOriginalTet, tetCenterIt->second));
            }
            else { // if doesn't exist, use average of edge cuts or the center
                if (ce.numPieces() == 4) {
                    averageEdgeWeight.fill(0.25);
                }
                else {
                    averageEdgeWeight = divide<T, 4>(averageEdgeWeight, cutEdges);
                }
                newNodes.push_back(elementCenter<T, 3>(tetMesh.nodes_, sortedOriginalTet, averageEdgeWeight));
                intersections[sortedOriginalTet] = averageEdgeWeight;
            }

            // add elements that are created by the new nodes added above
            vector<I4> newTets;
            for (int f = 0; f < faces.size(); ++f) {
                const auto& face = faces[f];
                const auto sortedFace = toI4<int, 3>(sorted(face));
                const auto& newFaceCenterIt = newNodeMapping.find(sortedFace);
                if (newFaceCenterIt != newNodeMapping.end()) {
                    for (int j = 0; j < 3; ++j) {
                        auto sortedEdge = toI4<int, 2>(sorted(array<int, 2>{face[j], face[(j + 1) % 3]}));
                        const auto& newEdgeCenterIt = newNodeMapping.find(sortedEdge);
                        if (newEdgeCenterIt != newNodeMapping.end()) {
                            if (ce.subElements[FaceIndexes[f][j]]) {
                                newTets.push_back(I4{ tetCenterId, newFaceCenterIt->second, face[j], newEdgeCenterIt->second });
                            }
                            if (ce.subElements[FaceIndexes[f][(j + 1) % 3]]) {
                                newTets.push_back(I4{ tetCenterId, newFaceCenterIt->second, newEdgeCenterIt->second, face[(j + 1) % 3] });
                            }
                        }
                        else if (ce.subElements[FaceIndexes[f][j]]) {
                            newTets.push_back(I4{ tetCenterId, newFaceCenterIt->second, face[j], face[(j + 1) % 3] });
                        }
                    }
                }
                else if (ce.subElements[FaceIndexes[f][0]]) { // no face intersection, might have 0 or 1 edge cut
                    bool isSplit = false;
                    for (int j = 0; j < 3; ++j) {
                        auto sortedEdge = toI4<int, 2>(sorted(array<int, 2>{face[j], face[(j + 1) % 3]}));
                        const auto& newEdgeCenterIt = newNodeMapping.find(sortedEdge);
                        if (newEdgeCenterIt != newNodeMapping.end()) {
                            newTets.push_back(I4{ tetCenterId, face[(j + 2) % 3], face[j], newEdgeCenterIt->second });
                            newTets.push_back(I4{ tetCenterId, face[(j + 2) % 3], newEdgeCenterIt->second, face[(j + 1) % 3] });
                            isSplit = true;
                            break;
                        }
                    }
                    if (!isSplit) {
                        newTets.push_back(I4{ tetCenterId, face[0], face[1], face[2] });
                    }
                }
            }
            newMesh[i] = newTets[0];
            for (int j = 1; j < newTets.size(); ++j) {
                newMesh.push_back(newTets[j]);
            }
        }
        return TetMesh<T>(move(newNodes), move(newMesh));
    }

public:
    static TetMesh<T> run(const TetMesh<T>& tetMesh, const TriMesh<T>& triMesh) {
        TetBoundary2TetIds tetBoundary2TetIds;
        //Find Intersections
        auto intersections = computeIntersections(tetMesh, triMesh, tetBoundary2TetIds);
        set<int> cutTets;
        vector<CutElement> cutElements = split(tetMesh, intersections, tetBoundary2TetIds, cutTets);
        vector<T3> newNodes;
        vector<I4> newMesh;
        merge(cutElements, tetMesh, newNodes, newMesh, intersections);
        return subdivide(cutElements, tetMesh, newNodes, newMesh, intersections);
    }
};



struct Spring {
    int mass1;
    int mass2;
    double rest_length;
    double spring_const;
    Spring(int Mass1, int Mass2, double RestLength, double SpringConst) {
        mass1 = Mass1;
        mass2 = Mass2;
        rest_length = RestLength;
        spring_const = SpringConst;
    }
};

struct Node {
    FVector Position;
    FVector Velocity;
    double Mass;
    TArray<int32> ConnectedSprings;
    Node(FVector _Position, FVector _Velocity, double _Mass) {
        Position = _Position;
        Velocity = _Velocity;
        Mass = _Mass;
    }

};

struct FTetrahedron
{
    int32 Vertices[4];
    int32 VerticesMass[4];
    bool isIntersection;
    bool EdgeIntersection[6];
    //Store Location of Intersection 
    FVector3f EdgeIntersectionPoint[6];


    FTetrahedron()
    {
        for (int i = 0; i < 6; i++) {
            EdgeIntersection[i] = false;
        }
        
        for (int i = 0; i < 4; ++i)
        {
            Vertices[i] = -1;
        }
        isIntersection = false;
    }

    FTetrahedron(int32 A, int32 B, int32 C, int32 D)
    {
        for (int i = 0; i < 6; i++) {
            EdgeIntersection[i] = false;
        }
        EdgeIntersection[0] = true;
        isIntersection = true;
        Vertices[0] = A;
        Vertices[1] = B;
        Vertices[2] = C;
        Vertices[3] = D;
    }

    void ClearIntersectionInformation() {
        isIntersection = false;
        for (int i = 0; i < 6; i++) {
            EdgeIntersection[i] = false;
        }
        for (int i = 0; i < 6; i++) {
            EdgeIntersectionPoint[i] = FVector3f();
        }
    }

};


UCLASS()
class FATALITEPROJECT_API ACylinderActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACylinderActor();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UProceduralMeshComponent* UpperProceduralMeshComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UProceduralMeshComponent* LowerProceduralMeshComponent;
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UProceduralMeshComponent* CuttedSurfaceProceduralMeshComponent;
    //DynamicMesh
    TArray<FTetrahedron> Tetrahedra;
    FTetrahedron* PointTetrahedron;
    FVector ActiveTetraPoint;

    TArray<FVector> Vertices;
    TArray<int32> Triangles;

    bool IsCutted = false;
    bool IsOverlappingSword = false;
    TArray<FVector> CuttingSurfaceVertices;
    TArray<int32> CuttingSurfaceTriangles;

    FVector ps;
    FVector pe;
    FVector cs;
    FVector ce;

    float AccumulatedDeltaTime;
    TArray<Node> Nodes;
    TArray<Spring> Springs;
    TetMesh<double> tetMesh;
    TriMesh<double> triMesh;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh")
	class UStaticMeshComponent* CylinderMeshComponent;

	UFUNCTION(BlueprintCallable, Category = "Cylinder")
	void OnSwordBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION(BlueprintCallable, Category = "Cylinder")
	void OnSwordEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
    
    //LOAD TETRAHEDRON FILES
    bool LoadFaces(FString filepath, TArray<int32>& fv);
    bool LoadTetrahedrons(FString filepath, TArray<FTetrahedron>& tt);
    bool LoadNodes(FString filepath, TArray<FVector>& fv);
    bool LoadEdges(FString filepath);
    bool RayTriangleIntersect(const FVector& origin, const FVector& direction,
        const FVector& v0, const FVector& v1, const FVector& v2,
        float& t, float& u, float& v);
    void FindIntersectionTetrahedrons(FVector& PreviousStartVector, FVector& PreviousEndVector, FVector& CurrentStartVector, FVector& CurrentEndVector);
    bool SameSide(FVector v1, FVector v2, FVector v3, FVector v4, FVector p);
    bool PointInTetrahedron(FVector v1, FVector v2, FVector v3, FVector v4, FVector p);


    void AddNodes(Node n);
    void AddSprings(Spring s);
    void SimulatingMassSpringDamper(double dt);
    FVector Acceleration(int SizeStep,const FVector& pos = FVector::Zero(), const FVector& vel = FVector::Zero());
};


