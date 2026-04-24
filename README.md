# Impulse Generation Tools for Unreal

This project aims to make using convolution reverb in Unreal Engine as easy as possible.
The plugin can even make it's own impulse responses that you can automatically update after you make changes to your world.
The Impulses in this plugin can be generated for use with your own systems that just need impulse responses, or automatically managed

## Impulse Generation

Impulse Responses are generated through the IR_Generator Actor. It has a few different options like whether or not you would like it to generate when you bake or the amount of rays that are used when generating. The Impules are dynamically generated based on the map geometry.

### Atmospheric Absorption

If you would like to account for sound lost due to the air turn on atmospheric absorption. This uses the air temperature and humidity and calculates the best fitting low pass filter at each moment during the impulse.

### Surface Absorption

If you would like to account for sound lost due to it hitting different surfaces turn on surface absoption. This uses the surface aborption coefficients in the project settings to filter the audio based the refection of the material at different frequency bands.

## Runtime Impulse Based Convolution Managment

If you want to automacally manage the generated impulse reponses...

### Baking System

Go to window -> IR Generator to find the baking window. hitting the bake button generates impulses at all of the IR_Generators and then updates any convolution volumes that were linked to a generator.

### Convolution Volumes

Convolution volumes can be used to have an automatically setup Convolution Reverb system very easily. After you place down your volume you can either put in an impulse response manually (either from the plugin or from elseware) or link an IR Generator and bake it. There is a parameter to control how long it takes transisioning to and from that impulse, note that when going from volume to volume the times are averaged. You can also change the priority if you want to place several convolution volumes near each other. You can use an empty convolution volume with a high priority to "carve out" shapes from other volumes.
