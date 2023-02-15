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

--@fun meshBooleanShape convenience wrapper for simIGL.meshBoolean to operate on shapes directly, also accepting more than 2 shapes, as multiple args or as a single table arg
--@arg int handleA the handle of the first shape
--@arg int handleB the handle of the second shape
--@arg int op the operation (see simIGL.boolean_op)
--@ret int handleResult the handle of the resulting shape
function simIGL.meshBooleanShape(...)
    local arg={...}
    local op=table.remove(arg,#arg)
    if type(arg[1])=='table' then
        if #arg~=1 then error('incorrect number of arguments') end
        arg=arg[1]
    end
    local function compute(a,b)
        local function blendColor(a,b) return (0.5*(Vector(a)+Vector(b))):data() end
        local m=simIGL.meshBoolean(simIGL.getMesh(a),simIGL.getMesh(b),op)
        local edgesA=sim.getObjectInt32Param(a,sim.shapeintparam_edge_visibility)
        local edgesB=sim.getObjectInt32Param(b,sim.shapeintparam_edge_visibility)
        local h=sim.createMeshShape(1+2*edgesA*edgesB,math.pi/8,m.vertices,m.indices)
        local _,coladA=sim.getShapeColor(a,'',sim.colorcomponent_ambient_diffuse)
        local _,coladB=sim.getShapeColor(b,'',sim.colorcomponent_ambient_diffuse)
        sim.setShapeColor(h,'',sim.colorcomponent_ambient_diffuse,blendColor(coladA,coladB))
        local _,colspA=sim.getShapeColor(a,'',sim.colorcomponent_specular)
        local _,colspB=sim.getShapeColor(b,'',sim.colorcomponent_specular)
        sim.setShapeColor(h,'',sim.colorcomponent_specular,blendColor(colspA,colspB))
        local _,colemA=sim.getShapeColor(a,'',sim.colorcomponent_emission)
        local _,colemB=sim.getShapeColor(b,'',sim.colorcomponent_emission)
        sim.setShapeColor(h,'',sim.colorcomponent_emission,blendColor(colemA,colemB))
        return h
    end
    local result=compute(arg[1],arg[2])
    if #arg>2 then
        local toRemove={}
        for i=3,#arg do
            table.insert(toRemove,result)
            result=compute(result,arg[i])
        end
        sim.removeObjects(toRemove)
    end
    sim.reorientShapeBoundingBox(result,sim.handle_world)
    return result
end

return simIGL
