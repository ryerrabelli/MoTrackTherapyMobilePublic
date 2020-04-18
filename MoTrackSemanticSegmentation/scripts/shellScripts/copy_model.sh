model="MobileNetV1_2lite"
mkdir ../../training_logs/$model
gcloud compute scp --recurse instance-1:/home/motracktherapy/MoTrackTherapyMobile/MoTrackSemanticSegmentation/training_logs/$model/model.ckpt-50000.* ../../training_logs/$model
gcloud compute scp --recurse instance-1:/home/motracktherapy/MoTrackTherapyMobile/MoTrackSemanticSegmentation/training_logs/$model/checkpoint ../../training_logs/$model
gcloud compute scp --recurse instance-1:/home/motracktherapy/MoTrackTherapyMobile/MoTrackSemanticSegmentation/training_logs/$model/*.pbtxt ../../training_logs/$model

