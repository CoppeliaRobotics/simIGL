#include <Eigen/Dense>
#include <igl/copyleft/cgal/mesh_boolean.h>
#include <igl/copyleft/cgal/CSGTree.h>
#include <igl/random_points_on_mesh.h>
#include <igl/swept_volume.h>
#include <igl/exact_geodesic.h>
#include <igl/copyleft/cgal/closest_facet.h>
#include <igl/upsample.h>
#include <igl/centroid.h>
#include <igl/barycenter.h>
#include <igl/copyleft/tetgen/tetrahedralize.h>
#include <igl/copyleft/cgal/convex_hull.h>
#include <igl/copyleft/cgal/intersect_with_half_space.h>
#include <igl/volume.h>
#include <igl/boundary_facets.h>
#include <simPlusPlus/Plugin.h>
#include "config.h"
#include "plugin.h"
#include "stubs.h"

using namespace std;
using namespace Eigen;

class Plugin : public sim::Plugin
{
public:
    void onInit()
    {
        if(!registerScriptStuff())
            throw runtime_error("failed to register script stuff");

        setExtVersion("IGL Plugin");
        setBuildDate(BUILD_DATE);
    }

    void readMesh(MatrixXd &V, MatrixXi &F, const mesh &m)
    {
        if(m.vertices.size() % 3)
            throw runtime_error("vertices must have 3 * n values");
        if(m.indices.size() % 3)
            throw runtime_error("indices must have 3 * n values");

        V.resize(m.vertices.size() / 3, 3);
        for(size_t i = 0, k = 0; i < V.rows(); i++)
            for(size_t j = 0; j < V.cols(); j++)
                V(i, j) = m.vertices.at(k++);

        F.resize(m.indices.size() / 3, 3);
        for(size_t i = 0, k = 0; i < F.rows(); i++)
            for(size_t j = 0; j < F.cols(); j++)
                F(i, j) = m.indices.at(k++);
    }

    void writeMesh(const MatrixXd &V, const MatrixXi &F, mesh &m)
    {
        m.vertices.clear();
        for(size_t i = 0; i < V.rows(); i++)
            for(size_t j = 0; j < V.cols(); j++)
                m.vertices.push_back(V(i, j));

        m.indices.clear();
        for(size_t i = 0; i < F.rows(); i++)
            for(size_t j = 0; j < F.cols(); j++)
                m.indices.push_back(F(i, j));
    }

    void readTetMesh(MatrixXd &V, MatrixXi &T, const tetmesh &m)
    {
        if(m.vertices.size() % 3)
            throw runtime_error("vertices must have 3 * n values");
        if(m.indices.size() % 4)
            throw runtime_error("indices must have 4 * n values");

        V.resize(m.vertices.size() / 3, 3);
        for(size_t i = 0, k = 0; i < V.rows(); i++)
            for(size_t j = 0; j < V.cols(); j++)
                V(i, j) = m.vertices.at(k++);

        T.resize(m.indices.size() / 4, 4);
        for(size_t i = 0, k = 0; i < T.rows(); i++)
            for(size_t j = 0; j < T.cols(); j++)
                T(i, j) = m.indices.at(k++);
    }

    void writeTetMesh(const MatrixXd &V, const MatrixXi &T, tetmesh &m)
    {
        m.vertices.clear();
        for(size_t i = 0; i < V.rows(); i++)
            for(size_t j = 0; j < V.cols(); j++)
                m.vertices.push_back(V(i, j));

        m.indices.clear();
        for(size_t i = 0; i < T.rows(); i++)
            for(size_t j = 0; j < T.cols(); j++)
                m.indices.push_back(T(i, j));
    }

    template<typename T>
    void readGrid(Matrix<T, Dynamic, Dynamic> &m, const Grid<T> &g)
    {
        if(g.dims.size() != 2)
            throw sim::exception("grid must be a matrix");
        m.resize(g.dims[0], g.dims[1]);
        size_t k = 0;
        for(size_t i = 0; i < g.dims[0]; i++)
            for(size_t j = 0; j < g.dims[1]; j++)
                m(i, j) = g.data[k++];
    }

    template<typename T>
    void writeGrid(const Matrix<T, Dynamic, Dynamic> &m, Grid<T> &g)
    {
        g.dims.clear();
        g.dims.push_back(m.rows());
        g.dims.push_back(m.cols());
        g.data.clear();
        size_t k = 0;
        for(size_t i = 0; i < g.dims[0]; i++)
            for(size_t j = 0; j < g.dims[1]; j++)
                g.data.push_back(m(i, j));
    }

    template<typename T>
    void writeGrid(const SparseMatrix<T> &m, Grid<T> &g)
    {
        g.dims.clear();
        g.dims.push_back(m.rows());
        g.dims.push_back(m.cols());
        g.data.clear();
        size_t k = 0;
        for(size_t i = 0; i < g.dims[0]; i++)
            for(size_t j = 0; j < g.dims[1]; j++)
                g.data.push_back(m.coeff(i, j));
    }

    void writeGrid(const Eigen::VectorXi &v, Grid<int> &g)
    {
        g.dims.clear();
        g.dims.push_back(v.size());
        g.dims.push_back(1);
        g.data.clear();
        for(int i = 0; i < v.size(); i++)
            g.data.push_back(v(i));
    }

    void writeGrid(const Eigen::RowVectorXi &v, Grid<int> &g)
    {
        g.dims.clear();
        g.dims.push_back(1);
        g.dims.push_back(v.size());
        g.data.clear();
        for(int i = 0; i < v.size(); i++)
            g.data.push_back(v(i));
    }

    template<typename T>
    void readVector(Matrix<T, Dynamic, 1> &m, const std::vector<T> &v)
    {
        m.resize(v.size());
        for(size_t i = 0; i < v.size(); i++)
            m(i, 0) = v.at(i);
    }

    template<typename T>
    void writeVector(const Matrix<T, Dynamic, 1> &m, std::vector<T> &v)
    {
        v.clear();
        for(size_t i = 0; i < m.rows(); i++)
            v.push_back(m(i, 0));
    }

    void meshBoolean(meshBoolean_in *in, meshBoolean_out *out)
    {
        MatrixXd VA, VB, VC;
        MatrixXi FA, FB, FC;

        readMesh(VA, FA, in->a);
        readMesh(VB, FB, in->b);

        igl::MeshBooleanType t = igl::NUM_MESH_BOOLEAN_TYPES;
        switch(in->op)
        {
        case simigl_bool_op_union:
            t = igl::MESH_BOOLEAN_TYPE_UNION;
            break;
        case simigl_bool_op_intersection:
            t = igl::MESH_BOOLEAN_TYPE_INTERSECT;
            break;
        case simigl_bool_op_difference:
            t = igl::MESH_BOOLEAN_TYPE_MINUS;
            break;
        case simigl_bool_op_symmetric_difference:
            t = igl::MESH_BOOLEAN_TYPE_XOR;
            break;
        case simigl_bool_op_resolve:
            t = igl::MESH_BOOLEAN_TYPE_RESOLVE;
            break;
        }

        igl::copyleft::cgal::mesh_boolean(VA, FA, VB, FB, t, VC, FC);

        writeMesh(VC, FC, out->result);
    }

    void randomPointsOnMesh(randomPointsOnMesh_in *in, randomPointsOnMesh_out *out)
    {
        MatrixXd V;
        MatrixXi F;
        readMesh(V, F, in->m);
        MatrixXd B;
        MatrixXi FI;
        MatrixXd X;
        igl::random_points_on_mesh(in->n, V, F, B, FI, X);
        if(in->convertToWorldCoords)
        {
            for(int i = 0; i < B.rows(); i++)
            {
                B.row(i) =
                    B(i, 0)*V.row(F(FI(i), 0)) +
                    B(i, 1)*V.row(F(FI(i), 1)) +
                    B(i, 2)*V.row(F(FI(i), 2));
            }
        }
        writeGrid(B, out->b);
        writeGrid(FI, out->fi);
    }

    void sweptVolume(sweptVolume_in *in, sweptVolume_out *out)
    {
        MatrixXd V, SV;
        MatrixXi F, SF;
        readMesh(V, F, in->m);
        const auto & transform = [=](const double t) -> Affine3d
        {
            transformCallback_in in1;
            in1.t = t;
            transformCallback_out out1;
            transformCallback(in->_.scriptID, in->transformFunc.c_str(), &in1, &out1);
            Matrix4d tm;
            tm << out1.transform[0], out1.transform[1], out1.transform[2], out1.transform[3],
                  out1.transform[4], out1.transform[5], out1.transform[6], out1.transform[7],
                  out1.transform[8], out1.transform[9], out1.transform[10], out1.transform[11],
                  0, 0, 0, 1;
            Affine3d T(tm);
            return T;
        };
        igl::swept_volume(V, F, transform, in->timeSteps, in->gridSize, in->isoLevel, SV, SF);
        writeMesh(SV, SF, out->m);
    }

    void exactGeodesic(exactGeodesic_in *in, exactGeodesic_out *out)
    {
        MatrixXd V;
        MatrixXi F;
        readMesh(V, F, in->m);
        VectorXi VS, FS, VT, FT;
        readVector(VS, in->vs);
        readVector(FS, in->fs);
        readVector(VT, in->vt);
        readVector(FT, in->ft);
        VectorXd d;
        igl::exact_geodesic(V, F, VS, FS, VT, FT, d);
        writeVector(d, out->distances);
    }

    void uniqueEdgeMap(uniqueEdgeMap_in *in, uniqueEdgeMap_out *out)
    {
        MatrixXi F, E, uE;
        Eigen::VectorXi EMAP, uEC, uEE;
        readGrid(F, in->f);
        igl::unique_edge_map(F, E, uE, EMAP, uEC, uEE);
        writeGrid(E, out->e);
        writeGrid(uE, out->ue);
        writeGrid(EMAP, out->emap);
        writeGrid(uEC, out->uec);
        writeGrid(uEE, out->uee);
    }

    void closestFacet(closestFacet_in *in, closestFacet_out *out)
    {
        MatrixXd V, P;
        MatrixXi F;
        readMesh(V, F, in->m);
        readGrid(P, in->points);
        VectorXi I, R, S;
        readVector(I, in->indices);
        MatrixXi EMAP, uEC, uEE;
        readGrid(EMAP, in->emap);
        readGrid(uEC, in->uec);
        readGrid(uEE, in->uee);
        if(I.size() > 0)
            igl::copyleft::cgal::closest_facet(V, F, I, P, EMAP, uEC, uEE, R, S);
        else
            igl::copyleft::cgal::closest_facet(V, F, P, EMAP, uEC, uEE, R, S);
        writeVector(R, out->r);
        writeVector(S, out->s);
    }

    void upsample(upsample_in *in, upsample_out *out)
    {
        MatrixXd V, NV;
        MatrixXi F, NF;
        readMesh(V, F, in->m);
        igl::upsample(V, F, NV, NF, in->n);
        writeMesh(NV, NF, out->m);
    }

    static void divideTriangle(Vector3d va, Vector3d vb, Vector3d vc, int ia, int ib, int ic, double thres, std::vector<double> &newV, std::vector<int> &newF)
    {
        double lab = (va - vb).norm();
        double lac = (va - vc).norm();
        double lbc = (vb - vc).norm();
        double lmax = std::max(lab, std::max(lac, lbc));
        if(lmax < thres)
        {
            newF.push_back(ia);
            newF.push_back(ib);
            newF.push_back(ic);
            return;
        }
        if(lbc == lmax) { std::swap(va, vc); std::swap(ia, ic); }
        else if(lac == lmax) { std::swap(vb, vc); std::swap(ib, ic); }
        Vector3d vab = (va + vb) / 2;
        int iab = newV.size() / 3;
        newV.push_back(vab.x());
        newV.push_back(vab.y());
        newV.push_back(vab.z());
        divideTriangle(va, vab, vc, ia, iab, ic, thres, newV, newF);
        divideTriangle(vab, vb, vc, iab, ib, ic, thres, newV, newF);
    }

    void adaptiveUpsample(adaptiveUpsample_in *in, adaptiveUpsample_out *out)
    {
        MatrixXd V, NV;
        MatrixXi F, NF;
        readMesh(V, F, in->m);
        std::vector<double> newV;
        std::vector<int> newF;
        for(int i = 0; i < V.rows(); i++)
            for(int j = 0; j < 3; j++)
                newV.push_back(V(i, j));

        for(int i = 0; i < F.rows(); i++) {
            int ia = F(i, 0), ib = F(i, 1), ic = F(i, 2);
            Vector3d va = V.row(ia), vb = V.row(ib), vc = V.row(ic);
            divideTriangle(va, vb, vc, ia, ib, ic, in->threshold, newV, newF);
        }

        NV.resize(newV.size() / 3, 3);
        NF.resize(newF.size() / 3, 3);
        for(int i = 0, iV = 0; i < newV.size(); i += 3, iV++)
            for(int j = 0; j < 3; j++)
                NV(iV, j) = newV[i + j];
        for(int i = 0, iF = 0; i < newF.size(); i += 3, iF++)
            for(int j = 0; j < 3; j++)
                NF(iF, j) = newF[i + j];
        writeMesh(NV, NF, out->m);
    }

    void centroid(centroid_in *in, centroid_out *out)
    {
        MatrixXd V;
        MatrixXi F;
        readMesh(V, F, in->m);
        VectorXd c(3);
        igl::centroid(V, F, c, out->vol);
        writeVector(c, out->c);
    }

    void barycenter(barycenter_in *in, barycenter_out *out)
    {
        MatrixXd V, BC;
        MatrixXi F;
        readGrid(V, in->v);
        readGrid(F, in->f);
        igl::barycenter(V, F, BC);
        writeGrid(BC, out->bc);
    }

    void tetrahedralize(tetrahedralize_in *in, tetrahedralize_out *out)
    {
        MatrixXd V, TV;
        MatrixXi F, TT, TF;
        readMesh(V, F, in->m);
        out->result = igl::copyleft::tetgen::tetrahedralize(V, F, in->switches, TV, TT, TF);
        writeTetMesh(TV, TT, out->m);
        writeGrid(TF, out->tf);
    }

    void convexHull(convexHull_in *in, convexHull_out *out)
    {
        VectorXd V;
        readVector(V, in->points);
        int n = V.size();
        if(n % 3)
            throw runtime_error("points must have 3n values");
        MatrixXd W;
        MatrixXi G;
#if EIGEN_VERSION_AT_LEAST(3,4,0)
        auto Vr = V.reshaped<RowMajor>(n / 3, 3);
#else
        Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> Vr(n / 3, 3);
        Vr = Eigen::Map<Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>>(V.data(), n / 3, 3);
#endif
        igl::copyleft::cgal::convex_hull(Vr, W, G);
        writeMesh(W, G, out->m);
    }

    void intersectWithHalfSpace(intersectWithHalfSpace_in *in, intersectWithHalfSpace_out *out)
    {
        MatrixXd V, VC;
        MatrixXi F, FC;
        readMesh(V, F, in->m);
        VectorXd p, n;
        readVector(p, in->pt);
        readVector(n, in->n);
        VectorXi J;
        igl::copyleft::cgal::intersect_with_half_space(V, F, p, n, VC, FC, J);
        writeMesh(VC, FC, out->m);
        writeVector(J, out->j);
    }

    void volume(volume_in *in, volume_out *out)
    {
        MatrixXd V;
        MatrixXi T;
        readTetMesh(V, T, in->m);
        VectorXd vol;
        igl::volume(V, T, vol);
        writeVector(vol, out->vol);
    }

    void boundaryFacets(boundaryFacets_in *in, boundaryFacets_out *out)
    {
        MatrixXd V;
        MatrixXi T;
        readTetMesh(V, T, in->m);
        MatrixXi SF;
        igl::boundary_facets(T, SF);
        writeMesh(V, SF, out->m);
    }

    void faceCentroids(faceCentroids_in *in, faceCentroids_out *out)
    {
        MatrixXd V, C;
        MatrixXi F;
        readMesh(V, F, in->m);
        C.resize(F.rows(), 3);
        for(int i = 0; i < F.rows(); i++)
            for(int j = 0; j < 3; j++)
                C(i, j) = (V(F(i, 0), j) + V(F(i, 1), j) + V(F(i, 2), j)) / 3.;
        writeGrid(C, out->c);
    }

    void meshOctreeIntersection(meshOctreeIntersection_in *in, meshOctreeIntersection_out *out)
    {
        MatrixXd V, NV;
        MatrixXi F, NF;
        readMesh(V, F, in->m);
        std::vector<double> newV;
        std::vector<int> newF;
        for(int fi = 0; fi < F.rows(); fi++)
        {
            std::vector<Vector3d> v {
                V.row(F(fi, 0)),
                V.row(F(fi, 1)),
                V.row(F(fi, 2)),
            };
            auto tn = (v[0] - v[1]).cross(v[0] - v[2]).normalized();
            std::vector<double> pts;
            for(const auto &vi : v)
            {
                pts.push_back(vi.x());
                pts.push_back(vi.y());
                pts.push_back(vi.z());
            }
            if(0 < sim::checkOctreePointOccupancy(in->oc, 0, pts))
            {
                for(int k = -1; k <= 1; k += 2)
                {
                    for(const auto &vi : v)
                    {
                        newF.push_back(newV.size() / 3);
                        auto vo = vi + tn * k * 0.001;
                        newV.push_back(vo.x());
                        newV.push_back(vo.y());
                        newV.push_back(vo.z());
                    }
                }
            }
        }
        NV.resize(newV.size() / 3, 3);
        NF.resize(newF.size() / 3, 3);
        for(int i = 0, iV = 0; i < newV.size(); i += 3, iV++)
            for(int j = 0; j < 3; j++)
                NV(iV, j) = newV[i + j];
        for(int i = 0, iF = 0; i < newF.size(); i += 3, iF++)
            for(int j = 0; j < 3; j++)
                NF(iF, j) = newF[i + j];
        writeMesh(NV, NF, out->m);
    }
};

SIM_PLUGIN(Plugin)
#include "stubsPlusPlus.cpp"
