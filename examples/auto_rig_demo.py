
from pynocchio import auto_rig, Mesh
from pynocchio import skeletons


human_mesh = Mesh('data/sveta.obj')
human_skeleton = skeletons.HumanSkeleton()
human_skeleton.scale(0.7)
attach = auto_rig(human_skeleton, human_mesh)

print('vertices:', len(human_mesh.vertices))
print('full_weights:', len(attach.full_weights))
print('bones_per_vertex:', len(attach.bones_per_vertex))
