import KratosMultiphysics as KM
import KratosMultiphysics.MappingApplication as KratosMapping

def ReadModelPart(model_part, mdpa_file_name):
    # adding variables used for mapping
    historical_vars = [KM.PRESSURE, KM.FORCE, KM.TEMPERATURE, KM.VELOCITY, KM.DISPLACEMENT]
    for var in historical_vars:
        model_part.AddNodalSolutionStepVariable(var)

    import_flags = KM.ModelPartIO.READ | KM.ModelPartIO.SKIP_TIMER
    KM.ModelPartIO(mdpa_file_name, import_flags).ReadModelPart(model_part)


current_model = KM.Model()

# Read origin modelpart
model_part_origin = current_model.CreateModelPart("origin")
ReadModelPart(model_part_origin, "coupled_cantilever/domainA")
# Artificially create interface submodelpart
model_part_origin.CreateSubModelPart("interface")
originInterface = model_part_origin.GetSubModelPart("interface")
for nodeIndex in range(328,334):
    originInterface.AddNodes([nodeIndex])

# Read destination modelpart
model_part_destination = current_model.CreateModelPart("destination")
ReadModelPart(model_part_destination, "coupled_cantilever/domainB")
# Artificially create interface submodelpart
model_part_destination.CreateSubModelPart("interface")
destinationInterface = model_part_destination.GetSubModelPart("interface")
destinationInterface.AddNodes([10])
destinationInterface.AddNodes([6])
destinationInterface.AddNodes([3])
destinationInterface.AddNodes([1])

mapper_params = KM.Parameters("""{
    "mapper_type": "coupling_geometry",
    "echo_level" : 0,
    "dual_mortar": true,
    "consistency_scaling" : false,
    "modeler_name" : "MappingGeometriesModeler",
    "modeler_parameters":{
        "origin_model_part_name" : "origin",
        "destination_model_part_name" : "destination",
        "is_interface_sub_model_parts_specified" : true,
        "origin_interface_sub_model_part_name" : "interface",
        "destination_interface_sub_model_part_name" : "interface"
    }
}""")

# ======== Left in here because this is a temporary technique to construct coupling geoms =======
# Create dummy line conditions on submodel parts to couple together
originProps = originInterface.GetProperties()[1]
nodeOffset = 327
for conditionIndex in range(1,6):
    cNode = nodeOffset + conditionIndex
    originInterface.CreateNewCondition("LineCondition2D2N", conditionIndex+100, [cNode,cNode+1], originProps)
destProps = destinationInterface.GetProperties()[1]
destinationInterface.CreateNewCondition("LineCondition2D2N", 101, [1,3], destProps)
destinationInterface.CreateNewCondition("LineCondition2D2N", 102, [3,6], destProps)
destinationInterface.CreateNewCondition("LineCondition2D2N", 103, [6,10], destProps)
# ===============================================================================================



mapper = KratosMapping.MapperFactory.CreateMapper(model_part_origin, model_part_destination, mapper_params)
#mapper = KratosMapping.MapperFactory.CreateMapper(current_model, mapper_params) # TODO @phil could you add this to the mapper_factory please






# Small test. The energy error should be zero
prescribed_displacement = 2.5
prescribed_force = 3.0
#print("\nMapping uniform displacement ",prescribed_displacement," from origin to destination")
for node in originInterface.Nodes:
    node.SetSolutionStepValue(KM.DISPLACEMENT_X, prescribed_displacement)

mapper.Map(KM.DISPLACEMENT, KM.DISPLACEMENT)
#print("destination interface nodes displacement values")
#for node in destinationInterface.Nodes:
#    print("\t",node.Id," displacement = ",node.GetSolutionStepValue(KM.DISPLACEMENT_X))
#    self.assertAlmostEqual(node.GetSolutionStepValue(KM.DISPLACEMENT_X),prescribed_displacement)

#print("\n\nInverse mapping from destination to origin")
for node in destinationInterface.Nodes:
    node.SetSolutionStepValue(KM.FORCE_X, prescribed_force)
    if node.Id == 1 or node.Id == 10:
        node.SetSolutionStepValue(KM.FORCE_X, prescribed_force/2.0)
    #print("\t",node.Id," displacement = ",node.GetSolutionStepValue(KM.FORCE_X))
    #print("\t",node.Id," Y position = ",node.Y)

mapper.InverseMap(KM.FORCE, KM.FORCE, KratosMapping.Mapper.USE_TRANSPOSE)
#print("destination interface nodes displacement values")

#for node in originInterface.Nodes:
#    print("\t",node.Id," forces = ",node.GetSolutionStepValue(KM.FORCE_X))
#    print("\t",node.Id," forces = ",node.Y)

#print("\n\nCHECKING ENERGY BALANCE")
energy_origin = 0.0
#print("Origin part")
for node in originInterface.Nodes:
    #print("\t",node.Id,"\n\t\tforces = ",node.GetSolutionStepValue(KM.FORCE_X),"\n\t\tdisp = ",node.GetSolutionStepValue(KM.DISPLACEMENT_X))
    energy_origin += node.GetSolutionStepValue(KM.FORCE_X) * node.GetSolutionStepValue(KM.DISPLACEMENT_X)

energy_dest = 0.0
#print("Destination part")
for node in destinationInterface.Nodes:
    #print("\t",node.Id,"\n\t\tforces = ",node.GetSolutionStepValue(KM.FORCE_X),"\n\t\tdisp = ",node.GetSolutionStepValue(KM.DISPLACEMENT_X))
    energy_dest += node.GetSolutionStepValue(KM.FORCE_X) * node.GetSolutionStepValue(KM.DISPLACEMENT_X)

print("Energy origin = ",energy_origin)
print("Energy destination = ",energy_dest)
print("Energy error = ",energy_origin-energy_dest)
#self.assertAlmostEqual(energy_origin, energy_dest)