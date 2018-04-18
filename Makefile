
all:

include ../kaldi.mk

TESTFILES =

OBJFILES = audio-buffer-source.o online-decoder.o speech-recognition-engine.o

LIBNAME = onlinedecoder

EXTRA_CXXFLAGS += $(shell pkg-config --cflags jansson)
EXTRA_LDLIBS += $(shell pkg-config --libs jansson)

ADDLIBS = ../online2/kaldi-online2.a ../ivector/kaldi-ivector.a \
          ../nnet3/kaldi-nnet3.a ../chain/kaldi-chain.a ../nnet2/kaldi-nnet2.a \
          ../cudamatrix/kaldi-cudamatrix.a ../decoder/kaldi-decoder.a \
          ../lat/kaldi-lat.a ../fstext/kaldi-fstext.a ../hmm/kaldi-hmm.a \
          ../feat/kaldi-feat.a ../transform/kaldi-transform.a \
          ../gmm/kaldi-gmm.a ../tree/kaldi-tree.a ../util/kaldi-util.a \
          ../matrix/kaldi-matrix.a \
          ../base/kaldi-base.a




include ../makefiles/default_rules.mk

