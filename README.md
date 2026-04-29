# Impulse Generation Tools for Unreal

This project aims to make using convolution reverb in Unreal Engine as easy as possible.
The plugin can even make it's own impulse responses that automatically update after you make changes to your world.
The Impulses in this plugin can be generated for use with your own systems that just need impulse responses (like if you wanted to use a convolution plugin in your DAW), or automatically managed to be used in an unreal engine game.

## Impulse Generation

Impulse Responses are generated through the IR_Generator Actor. It has a few different options like whether or not you would like it to generate when you bake or the amount of rays that are used when generating. The Impules are dynamically generated based on the map geometry.

**Note**: For accurate results, target meshes must allow Visibility line traces against the full mesh surface. If traces miss walls or hit incorrect locations, check the mesh collision settings and enable complex collision tracing.

The easiest way to do this is to change the default shape complexity in the project's physics settings to 'Use Complex As Simple', at least while generating.

### Atmospheric Absorption

If you would like to account for sound lost due to the air turn on atmospheric absorption. This uses the air temperature and humidity and calculates the best fitting low pass filter at each moment during the impulse.

### Surface Absorption

If you would like to account for sound lost due to it hitting different surfaces turn on surface absoption. This uses the surface aborption coefficients in the project settings to filter the audio based the refection of the material at different frequency bands.

### Settings Window

There is a settings menu in your project settings to manage the air temperature and humidity used for the calculations, as well as the reflectiveness of different surfaces at specific frequency bands.

## Runtime Impulse Based Convolution Managment

If you want to automacally manage the generated impulse reponses that you have made with this plugin, the easiest way to do that is to use the Convolution Volume and baking sysstem that come with the plugin.

### Convolution Volumes

Convolution volumes can be used to have an automatically setup Convolution Reverb system very easily. After you place down your volume you can either put in an impulse response manually (either from the plugin or from elseware) or link an IR Generator and bake it. There is a parameter to control how long it takes transisioning to and from that impulse, note that when going from volume to volume the times are averaged. You can also change the priority if you want to place several convolution volumes near each other. You can use an empty convolution volume with a high priority to "carve out" shapes from other volumes.

### Baking System

Go to window -> IR Generator to find the baking window. hitting the bake button generates impulses at all of the IR_Generators and then updates any convolution volumes that were linked to a generator.

# Setup Video

Getting a minimal setup of the plugin is simple. Here is me setting up the unreal shooter variant demo to use convulution reverbs. The only modifications were placing the room and adding a sound to the pistol shooting. The plugin was added directly to the ProjectRoot/Plugins folder.

Some Parts of the video are sped up / cut out for time.
