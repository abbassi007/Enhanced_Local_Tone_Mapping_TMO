import os
import numpy as np
# Image writing
# from PIL import Image
# Image loading
import rawpy
import imageio

def load_exr(p):
	imageio.plugins.freeimage.download()
	hdr_img = imageio.imread(p, format='EXR-FI')
	hdr_luma = np.average(hdr_img, axis=2, weights=[0.2125, 0.7154, 0.0721])
	if np.max(hdr_luma) > 1.0:
		mean_luma = np.mean(hdr_luma)
		max_luma = np.max(hdr_luma)
		auto_exp = min(9.6 * mean_luma, max_luma)
		hdr_img = hdr_img / auto_exp
	# hdr_img = np.power(hdr_img, 1. / 2.2)
	# hdr_img = np.clip(hdr_img, 0., 1.)
	return hdr_img

def load_raw(p):
	raw_img = rawpy.imread(p)
	rgb_img = raw_img.postprocess(use_camera_wb=True,
	                              half_size=False,
								  no_auto_bright=False,
								  output_bps=16,
								  output_color=rawpy.ColorSpace.sRGB,
								  highlight_mode=rawpy.HighlightMode.Blend)
	rgb_img = rgb_img.astype(np.float32) / 65535.
	rgb_img = np.power(rgb_img, 2.2) # To linear color space
	return rgb_img

def load_image(input):
	ext = os.path.splitext(input)[-1]
	if ext == '.exr':
		return load_exr(input)
	if ext == '.dng':
		return load_raw(input)
	print(f'Image of type {ext} is not supported')
	exit(-1)

def main(input, output):
	img = load_image(input)
	img = np.clip(img, 0., 1.)
	# img = (img * 65535).astype(np.uint16)
	out_path = os.path.splitext(output)
	if out_path[-1] != '.pfm':
		output = out_path[0] + '.pfm'
	imageio.imsave(output, img)

if __name__=='__main__':
	import sys
	try:
		main(sys.argv[1], sys.argv[2])
	except Exception as e:
		print(e)
		print(f'Usage: {sys.argv[0]} INPUT OUTPUT')