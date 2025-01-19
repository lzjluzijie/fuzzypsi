# Fuzzy PSI

## Build

This project depends on `boost` and `libOTe`. The author used `gcc 11.5.0`.

```shell
git clone https://github.com/lzjluzijie/fuzzypsi.git --recursive
cd libOTe
python build.py --all --boost --sodium
cd .. 
mkdir build
cd build
cmake ..
make -j
```

To run it:

```shell
./fuzzypsi bench C 2048 2048 5 1000 129
```

## Contact

Contact `lzjluzijie@gmail.com` if you have any question.
