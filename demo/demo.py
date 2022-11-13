#!/usr/bin/env python

from PyQt6.QtCore import *
from PyQt6.QtWidgets import *
from PyQt6.QtGui import *
import os, subprocess, math, sys
import urllib.request
import numpy as np
import cv2
import rawpy
import imageio
imageio.plugins.freeimage.download()
import imageio.v2 as imageio

class EltmTmo:
    def __init__(self):
        wd = os.getcwd()
        print(f'Backup working dir: {wd}')
        os.chdir('..')
        os.makedirs('build_cmake', exist_ok=True)
        os.chdir('build_cmake')
        subprocess.check_call(['cmake', '-DCMAKE_BUILD_TYPE=Release', '..'])
        subprocess.check_call(['cmake', '--build', '.', '--config', 'Release'])
        os.chdir(wd)
        if sys.platform=='win32':
            self.bin_path = os.path.join('..', 'build_cmake', 'Release', 'ELTM-TMO.exe')
        elif sys.platform=='linux':
            self.bin_path = os.path.join('..', 'build_cmake', 'ELTM-TMO')
        assert os.path.exists(self.bin_path)
        self.cache_dir = 'pfm_cache'
        os.makedirs(self.cache_dir, exist_ok=True)

    def process(self, img, name, p=1, k=1, m=-2.7, s=1, etaF=1, etaC=1.5, lambdaF=0.02, lambdaC=1, BPCmax=0.9, BPCmin=0.1, rS=3, tauR=5):
        cache_path = os.path.join(self.cache_dir, name + '.pfm')
        if not os.path.exists(cache_path):
            img_lin = np.power(img, 2.2)              # Linearize
            img_lin = np.clip(img_lin, 0., 1.)        # Clip
            imageio.imsave(cache_path, img_lin)
        output_path = os.path.join(self.cache_dir, name + '.ppm')
        # print(f'Using params: p={p}, k={k}, m={m}, s={s}, etaF={etaF}, etaC={etaC}, lambdaF={lambdaF}, lambdaC={lambdaC}, BPCmax={BPCmax}, BPCmin={BPCmin}, rS={rS}, tauR={tauR}')
        subprocess.check_call([self.bin_path, 
                               cache_path, 
                               output_path, 
                               '-p', str(p), 
                               '-k', str(k), 
                               '-m', str(m), 
                               '-s', str(s),
                               '-eta-f', str(etaF),
                               '-eta-c', str(etaC),
                               '-lambda-f', str(lambdaF),
                               '-lambda-c', str(lambdaC),
                               '-bpc-max', str(BPCmax),
                               '-bpc-min', str(BPCmin),
                               '-rs', str(rS),
                               '-tau-r', str(tauR)])
        img_out = imageio.imread(output_path)
        return img_out.astype(np.float32) / 255.

def get_rgb_raw(img):
    return img.postprocess(use_camera_wb=True, 
                           half_size=False, 
                           output_bps=16, 
                           no_auto_bright=False,
                           output_color=rawpy.ColorSpace.sRGB, 
                           highlight_mode=rawpy.HighlightMode.Blend)

def load_raw(raw_img_path):
    raw = rawpy.imread(raw_img_path)
    rgb = get_rgb_raw(raw)
    return np.clip(rgb / 65535., 0., 1.).astype(np.float32)

def load_jpg(jpg_img_path):
    img = imageio.imread(jpg_img_path)
    return np.clip(img / 255., 0., 1.).astype(np.float32)

def download_file(url, path):
    urllib.request.urlretrieve(url, path)

def npimage_to_qpixmap(img):
    img_8bit = (img * 255).astype(np.uint8)
    height, width, channel = img_8bit.shape
    bytes_per_line = channel * width
    return QPixmap(QImage(img_8bit.data, width, height, bytes_per_line, QImage.Format.Format_RGB888))

def downsize_preview(img, max_dim=640):
    # Resize image preview for performance
    if img.shape[0] > img.shape[1]:
        h = max_dim
        w = int((float(h) / float(img.shape[0])) * img.shape[1])
    else:
        w = max_dim
        h = int((float(w) / float(img.shape[1])) * img.shape[0])
    ret = cv2.resize(img, dsize=(w, h), interpolation=cv2.INTER_CUBIC)
    if ret.dtype==float:
        return np.clip(ret, 0.0, 1.0)
    return np.clip(ret, 0, 255)

class HdrPlusFetcher:
    def __init__(self):
        with open("hdrplus.txt") as f:
            self.img_names = f.readlines()
        print(f'HDR+ has {len(self.img_names)} images')
        # Strip newline from names
        self.img_names = [x.strip() for x in self.img_names]
        self.cache_dir = 'hdr+_cache'
        os.makedirs(self.cache_dir, exist_ok=True)

    def __len__(self):
        return len(self.img_names)

    def get_image_by_index(self, index):
        cp_in, cp_out = self.get_cache_path(index)
        img_name = self.img_names[index]
        if not os.path.exists(cp_in):
            url = f'https://storage.googleapis.com/hdrplusdata/20171106/results_20171023/{img_name}/merged.dng'
            download_file(url, cp_in)
        if not os.path.exists(cp_out):
            url = f'https://storage.googleapis.com/hdrplusdata/20171106/results_20171023/{img_name}/final.jpg'
            download_file(url, cp_out)
        img_in = load_raw(cp_in)
        img_out = load_jpg(cp_out)
        return downsize_preview(img_in), downsize_preview(img_out)

    def get_name(self, index):
        return self.img_names[index]

    def get_index(self, name):
        try:
            index = self.img_names.index(name)
            return index
        except:
            print('Image not found')
            return None

    def get_cache_path(self, index):
        name = self.img_names[index]
        return os.path.join(self.cache_dir, name + '.dng'), \
               os.path.join(self.cache_dir, name + '.jpg')

class Slider(QWidget):
    int_value_changed = pyqtSignal(int)

    def __init__(self, description, value, min, max):
        super().__init__()
        self.description = QLabel()
        self.description.setText(description)
        self.slider = QSlider()
        self.slider.setOrientation(Qt.Orientation.Horizontal)
        self.slider.setRange(min, max)
        self.slider.setValue(value)
        l = QHBoxLayout()
        l.addWidget(self.description)
        l.addWidget(self.slider)
        self.setLayout(l)

        self.slider.sliderReleased.connect(self.emit_int_value_changed)

    def emit_int_value_changed(self):
        self.int_value_changed.emit(self.get_value())

    def get_value(self):
        return self.slider.value()

    def get_description(self):
        return self.description.text()

class FloatSlider(Slider):
    # create our our signal that we can connect to if necessary
    float_value_changed = pyqtSignal(float)

    def __init__(self, description, value, min, max, step):
        self.min = min
        self.max = max
        self.step = step
        self.num_steps = int(math.ceil((max - min) / step))
        # print(f'{description} has {self.num_steps} steps')
        int_value = int(((value - self.min) / (self.max - self.min)) * self.num_steps)
        # print(f'[{self.min} -- {value} -- {self.max}]')
        # print(f'[0 -- {int_value} -- {self.num_steps}]')
        super().__init__(description, int_value, 0, self.num_steps)

        self.slider.sliderReleased.disconnect(super().emit_int_value_changed)
        self.slider.sliderReleased.connect(self.emit_float_value_changed)

    def emit_float_value_changed(self):
        self.float_value_changed.emit(self.get_value())

    def get_value(self):
        int_value = super().get_value()
        t = float(int_value) / float(self.num_steps)
        # print(f'{super().get_description()}:')
        # print(f'[0 -- {int_value} -- {self.num_steps}')
        float_value = self.max * t + self.min * (1 - t)
        # print(f'[0 -- {t} -- 1]')
        # print(f'[{self.min} -- {float_value} -- {self.max}')
        return float_value

class EltmParams(QWidget):
    param_value_changed = pyqtSignal()

    def __init__(self):
        super().__init__()
        columns = []
        for i in range(3):
            columns.append(QVBoxLayout())
        self.rS = Slider('rS', 3, 1, 100) 
        self.lambdaF = FloatSlider('λF', 0.02, 0.0, 2.0, 0.001)
        self.lambdaC = FloatSlider('λC', 1, 0.0, 2.0, 0.001)
        self.tauR = Slider('τR', 5, 1, 100)
        self.etaF = FloatSlider('ηF', 1, 0.0, 10.0, 0.001)
        self.etaC = FloatSlider('ηC', 1.5, 0.0, 10.0, 0.001)
        self.p = FloatSlider('p', 1, 0.0, 10.0, 0.001)
        self.k = FloatSlider('k', 1, 0.0, 10.0, 0.001)
        self.m = FloatSlider('m', -2.7, -5.0, 10.0, 0.001)
        self.BPCmax = FloatSlider('BPCmax', 0.9, 0., 1.0, 0.001)
        self.BPCmin = FloatSlider('BPCmin', 0.1, 0., 1.0, 0.001)
        self.s = FloatSlider('s', 1, 0., 10.0, 0.001)

        # Connect signals
        self.rS.int_value_changed.connect(self.emit_param_changed)
        self.lambdaF.float_value_changed.connect(self.emit_param_changed)
        self.lambdaC.float_value_changed.connect(self.emit_param_changed)
        self.tauR.int_value_changed.connect(self.emit_param_changed)
        self.etaF.float_value_changed.connect(self.emit_param_changed)
        self.etaC.float_value_changed.connect(self.emit_param_changed)
        self.p.float_value_changed.connect(self.emit_param_changed)
        self.k.float_value_changed.connect(self.emit_param_changed)
        self.m.float_value_changed.connect(self.emit_param_changed) 
        self.BPCmax.float_value_changed.connect(self.emit_param_changed)  
        self.BPCmin.float_value_changed.connect(self.emit_param_changed) 
        self.s.float_value_changed.connect(self.emit_param_changed)

        # Column 1
        columns[0].addWidget(self.s)
        columns[0].addWidget(self.p)
        columns[0].addWidget(self.k)
        columns[0].addWidget(self.m)
        # Column 2
        columns[1].addWidget(self.etaF)
        columns[1].addWidget(self.etaC)
        columns[1].addWidget(self.lambdaF)
        columns[1].addWidget(self.lambdaC)
        # Column 3
        columns[2].addWidget(self.BPCmax)
        columns[2].addWidget(self.BPCmin)
        columns[2].addWidget(self.rS)
        columns[2].addWidget(self.tauR)

        main_layout = QHBoxLayout()
        for col in columns:
            main_layout.addLayout(col)
        self.setLayout(main_layout)

    def emit_param_changed(self, _):
        self.param_value_changed.emit()

    def get_rS(self):
        return self.rS.get_value()
    def get_lambdaF(self):
        return self.lambdaF.get_value()
    def get_lambdaC(self):
        return self.lambdaC.get_value()
    def get_tauR(self):
        return self.tauR.get_value()
    def get_etaF(self):
        return self.etaF.get_value()
    def get_etaC(self):
        return self.etaC.get_value()
    def get_p(self):
        return self.p.get_value()
    def get_k(self):
        return self.k.get_value()
    def get_m(self):
        return self.m.get_value()
    def get_s(self):
        return self.s.get_value()
    def get_BPCmax(self):
        return self.BPCmax.get_value()
    def get_BPCmin(self):
        return self.BPCmin.get_value()

class MainWindow(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle('ELTM-TMO Demo')
        self.eltm = EltmTmo()
        self.hdrplus = HdrPlusFetcher()
        self.params = EltmParams()

        # Preview images
        self.img_index = 0
        self.current_image = None
        self.img_src = QPixmap()
        self.img_dest = QPixmap()
        self.img_eltm = QPixmap()
        self.label_src = QLabel()
        self.label_dest = QLabel()
        self.label_eltm = QLabel()

        # Layout
        preview_hdrplus = QVBoxLayout()
        preview_hdrplus.addWidget(self.label_src)
        preview_hdrplus.addWidget(self.label_dest)
        preview_grid = QHBoxLayout()
        preview_grid.addLayout(preview_hdrplus)
        preview_grid.addWidget(self.label_eltm)

        self.goto_img_by_index = QSpinBox()
        self.goto_img_by_index.setRange(0, len(self.hdrplus) - 1)
        self.goto_img_by_name = QLineEdit()
        search_layout = QHBoxLayout()
        search_layout.addWidget(self.goto_img_by_index)
        search_layout.addWidget(self.goto_img_by_name)

        main_layout = QVBoxLayout()
        main_layout.addWidget(self.params)
        main_layout.addLayout(preview_grid)
        main_layout.addLayout(search_layout)

        self.setLayout(main_layout)

        # Show window
        self.setGeometry(50, 50, 320, 200)
        self.show()

        # Connect signals
        self.params.param_value_changed.connect(self._on_param_change_)
        self.goto_img_by_index.valueChanged.connect(self._on_index_change_)
        self.goto_img_by_name.textChanged.connect(self._on_name_change_)

        # Make initial call manually
        self._on_index_change_()

    def _on_param_change_(self):
        self.img_eltm = npimage_to_qpixmap(self.process_eltm(self.current_image, self.hdrplus.get_name(self.img_index)))
        self.label_eltm.setPixmap(self.img_eltm)

    def _on_index_change_(self):
        self.img_index = self.goto_img_by_index.value()
        img_name = self.hdrplus.get_name(self.img_index)
        hdrplus_imgs = self.hdrplus.get_image_by_index(self.img_index)
        self.current_image = hdrplus_imgs[0]
        self.img_src = npimage_to_qpixmap(hdrplus_imgs[0]).scaled(QSize(320,240), Qt.AspectRatioMode.KeepAspectRatio) 
        self.img_dest = npimage_to_qpixmap(hdrplus_imgs[1]).scaled(QSize(320,240), Qt.AspectRatioMode.KeepAspectRatio)
        self.img_eltm = npimage_to_qpixmap(self.process_eltm(self.current_image, img_name))
        self.label_src.setPixmap(self.img_src)
        self.label_dest.setPixmap(self.img_dest)
        self.label_eltm.setPixmap(self.img_eltm)
        self.goto_img_by_name.setText(img_name)

    def _on_name_change_(self):
        new_index = self.hdrplus.get_index(self.goto_img_by_name.text())
        if new_index is None:
            # Restore original value
            self.goto_img_by_name.setText(self.hdrplus.get_name(self.img_index))
            return
        if new_index==self.img_index:
            # Do nothing
            return
        # Update index
        self.goto_img_by_index.setValue(new_index)

    def process_eltm(self, img, name):
        return self.eltm.process(img, name,                       
                                 self.params.get_p(),        
                                 self.params.get_k(),        
                                 self.params.get_m(),        
                                 self.params.get_s(),        
                                 self.params.get_etaF(),     
                                 self.params.get_etaC(),     
                                 self.params.get_lambdaF(),  
                                 self.params.get_lambdaC(),  
                                 self.params.get_BPCmax(),   
                                 self.params.get_BPCmin(),   
                                 self.params.get_rS(),
                                 self.params.get_tauR())

def main():
    app = QApplication([])
    window = MainWindow()
    app.exec()

if __name__=='__main__':
    main()