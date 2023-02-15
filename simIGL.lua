local simIGL={}

--@fun getMesh get mesh data of a given shape in the format used by simIGL functions
--@arg int h the handle of the shape
--@arg {type=table,default={}} options options
--@ret table mesh mesh object
function simIGL.getMesh(h,options)
    local v,i,n=sim.getShapeMesh(h)
    local m=sim.getObjectMatrix(h,-1)
    v=sim.multiplyVector(m,v)
    return {vertices=v, indices=i}
end

--@fun meshBooleanShape convenience wrapper for simIGL.meshBoolean to operate on shapes directly
--@arg table.int handles the handle of the input shapes
--@arg int op the operation (see simIGL.boolean_op)
--@ret int handleResult the handle of the resulting shape
function simIGL.meshBooleanShape(handles,op)
    if #handles<2 then error('not enough input shapes') end
    local function compute(a,b)
        local function blendColor(a,b) return (0.5*(Vector(a)+Vector(b))):data() end
        local m=simIGL.meshBoolean(simIGL.getMesh(a),simIGL.getMesh(b),op)
        local edgesA=sim.getObjectInt32Param(a,sim.shapeintparam_edge_visibility)
        local edgesB=sim.getObjectInt32Param(b,sim.shapeintparam_edge_visibility)
        local h=sim.createMeshShape(1+2*edgesA*edgesB,math.pi/8,m.vertices,m.indices)
        local _,coladA=sim.getShapeColor(a,nil,sim.colorcomponent_ambient_diffuse)
        local _,coladB=sim.getShapeColor(b,nil,sim.colorcomponent_ambient_diffuse)
        sim.setShapeColor(h,nil,sim.colorcomponent_ambient_diffuse,blendColor(coladA,coladB))
        local _,colspA=sim.getShapeColor(a,nil,sim.colorcomponent_specular)
        local _,colspB=sim.getShapeColor(b,nil,sim.colorcomponent_specular)
        sim.setShapeColor(h,nil,sim.colorcomponent_specular,blendColor(colspA,colspB))
        local _,colemA=sim.getShapeColor(a,nil,sim.colorcomponent_emission)
        local _,colemB=sim.getShapeColor(b,nil,sim.colorcomponent_emission)
        sim.setShapeColor(h,nil,sim.colorcomponent_emission,blendColor(colemA,colemB))
        return h
    end
    local result=compute(handles[1],handles[2])
    if #handles>2 then
        local toRemove={}
        for i=3,#handles do
            table.insert(toRemove,result)
            result=compute(result,handles[i])
        end
        sim.removeObjects(toRemove)
    end
    sim.reorientShapeBoundingBox(result,sim.handle_world)
    return result
end

return simIGL
