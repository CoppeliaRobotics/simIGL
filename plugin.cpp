#include <Eigen/Dense>
#include <igl/copyleft/cgal/mesh_boolean.h>
#include <igl/copyleft/cgal/CSGTree.h>
#include <igl/random_points_on_mesh.h>
#include "simPlusPlus/Plugin.h"
#include "config.h"
#include "plugin.h"
#include "stubs.h"

using namespace std;
using namespace Eigen;

class Plugin : public sim::Plugin
{
public:
    void onStart()
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
        if(!m.normals)
            sim::addLog(sim_verbosity_warnings, "normals not specified");

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

    void meshBoolean(meshBoolean_in *in, meshBoolean_out *out)
    {
        MatrixXd VA, VB, VC;
        MatrixXi FA, FB, FC;

        readMesh(VA, FA, in->a);
        readMesh(VB, FB, in->b);

        igl::MeshBooleanType t = igl::NUM_MESH_BOOLEAN_TYPES;
        switch(in->op)
        {
        case sim_igl_bool_op_union:
            t = igl::MESH_BOOLEAN_TYPE_UNION;
            break;
        case sim_igl_bool_op_intersection:
            t = igl::MESH_BOOLEAN_TYPE_INTERSECT;
            break;
        case sim_igl_bool_op_difference:
            t = igl::MESH_BOOLEAN_TYPE_MINUS;
            break;
        case sim_igl_bool_op_symmetric_difference:
            t = igl::MESH_BOOLEAN_TYPE_XOR;
            break;
        case sim_igl_bool_op_resolve:
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
        igl::random_points_on_mesh(in->n, V, F, B, FI);
        writeGrid(B, out->b);
        writeGrid(FI, out->fi);
    }
};

SIM_PLUGIN(PLUGIN_NAME, PLUGIN_VERSION, Plugin)
#include "stubsPlusPlus.cpp"
