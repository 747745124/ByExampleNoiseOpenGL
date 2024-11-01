## An implementation (WIP) of the Paper "High Performance By Example Noise"

#### Notes:
* A quick sketch of the paper is: input (low-res texture) -gaussianize-> gaussian_image -blending-> blended gaussian -inverse_gaussianize-> high-res texture. So ideally you can get an infinite resolution texture with similar histogram as the input.
  
* The implementation is currently an offline solution. I use PyOT to gaussianize the input texture, OpenGL (GLSL) to perform the blend and PyOT to perform the inverse transform.

* The LUT real-time solution is currently under investigation (bugged for now)

* The dependencies of python environment is under `./requirements.txt`

  * To use (Python version 3.9.6)

  * ```shell
    pip install -r requirements.txt
    ```

* The dependencies of OpenGL environments are listed in `CMakeLists.txt`

* Choose from different blending method. (LUT method unavailable)

  * Press space to hide GUI
  * Press tab to take a screenshot (as the input for inverse transform), the screenshot will be saved to your input path

  <img src="https://s2.loli.net/2024/11/01/yYjBHN9mLT3gvpe.png" alt="image-20241031193907917" style="zoom:50%;" />

* Use screenshot from OpenGL blending (click Use Gaussian to get the blended gaussian image), feed to the `inverse_gaussianize.py` file to get the final result.
* If you’re only interested in the shader blending process, check this out:
  * https://www.shadertoy.com/view/MdyfDV



#### Bugs / Artifacts of Implementation 

* Most of the values after Gaussian->Uniform transform are within the range [0.467, 0.533], if we use a lut on the range of [0,1], we might get most of our pixels mapped to the same color. And thus making the results having very low contrast.

<img src="https://s2.loli.net/2024/11/01/tryGeNOMw8IPSFp.png" alt="Screenshot 2024-10-29 at 21.54.20" style="zoom:33%;" />

* The blending might gets horizontal or vertical artifacts for some reason (i.e. not seamless).

<img src="https://s2.loli.net/2024/11/01/ZVgaOsDTA9rflbW.png" alt="image-20241031205318594" style="zoom:33%;" />

* The Optimal Transport method in my implementation makes the final histogram look squiggly (see results below).



#### Files:

* `image_scaler` 

  * Simple image scale function

* `image_visualizer`

  * Image visualization functions including histograms and difference between images

* `lut_visualizer`

  * A tool to visualize LUT

* `gaussianize`

  * Gaussianize the input image using optimal transport. (See 4.1 / 4.2 of the paper)
    * Use a batched implementation to avoid OOM.

* `inverse_gaussianize` (WIP)

  * Inverse transform the gaussianized image to input-like texture.
    * Current implementation uses optimal transport (bug exists for LUT solution)

* `./shader/synth.fs`

  * Simplex interpolation 

  

#### Results:

![image-20241031173213727](https://s2.loli.net/2024/11/01/9LPFIRrio5hez87.png)

#### ![image-20241031175246453](https://s2.loli.net/2024/11/01/m7qSXscVNyD6grH.png)

![image-20241031171948642](https://s2.loli.net/2024/11/01/nb7AZWKaJ3RDiIX.png)

![image-20241031174225608](https://s2.loli.net/2024/11/01/pz92ctyvwEngoRB.png)
