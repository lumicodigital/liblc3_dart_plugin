import 'dart:ffi';
import 'dart:typed_data';

import 'package:ffi/ffi.dart';

enum LC3DecoderError {
  lc3DecoderOk(0),
  lC3DecoderNotInitialized(1),
  lC3DecoderInvalidParams(2),
  lC3DecoderMemoryError(3),
  lc3DecoderInternalError(4);

  final int value;

  const LC3DecoderError(this.value);

  static LC3DecoderError fromValue(int value) {
    return values.firstWhere(
      (element) => element.value == value,
      orElse: () => throw ArgumentError("Invalid value: $value"),
    );
  }
}

typedef DecoderInitC = Int32 Function(Uint8, Uint8, Uint32, Uint16);
typedef DecoderInitDart = int Function(int, int, int, int);

typedef DecoderDecodeC = Int32 Function(Pointer<Uint8>, Pointer<Int16>);
typedef DecoderDecodeDart = int Function(Pointer<Uint8>, Pointer<Int16>);

typedef DecoderDestroyC = Void Function();
typedef DecoderDestroyDart = void Function();

typedef DecoderGetBlockBytesC = Int32 Function();
typedef DecoderGetBlockBytesDart = int Function();

typedef DecoderGetFrameSamplesC = Int32 Function();
typedef DecoderGetFrameSamplesDart = int Function();

class LC3WrapperDecoder {
  bool _isInitialized = false;
  late final DynamicLibrary _lc3Internal;
  late final DecoderInitDart _decoderInit;
  late final DecoderDecodeDart _decoderDecode;
  late final DecoderDestroyDart _decoderDestroy;
  late final DecoderGetBlockBytesDart _decoderGetBlockBytes;
  late final DecoderGetFrameSamplesDart _decoderGetFrameSamples;

  Pointer<Uint8> _encodedData = nullptr;
  int _encodedDataSize = 0;
  Pointer<Int16> _decodedData = nullptr;
  int _decodedDataSize = 0;

  LC3WrapperDecoder._private();

  static final LC3WrapperDecoder _instance = LC3WrapperDecoder._private();

  static LC3WrapperDecoder get instance => _instance;

  void checkInitialized() {
    if (!_isInitialized) {
      throw Exception("LC3WrapperDecoder is not initialized");
    }
  }

  Future<void> initialize(String assetPath) async {
    if (_isInitialized) {
      return;
    }

    _lc3Internal = DynamicLibrary.open(assetPath);
    _decoderInit = _lc3Internal
        .lookupFunction<DecoderInitC, DecoderInitDart>("decoder_init");
    _decoderDecode = _lc3Internal
        .lookupFunction<DecoderDecodeC, DecoderDecodeDart>("decoder_decode");
    _decoderDestroy = _lc3Internal
        .lookupFunction<DecoderDestroyC, DecoderDestroyDart>("decoder_destroy");
    _decoderGetBlockBytes = _lc3Internal.lookupFunction<DecoderGetBlockBytesC,
        DecoderGetBlockBytesDart>("decoder_get_block_bytes");
    _decoderGetFrameSamples = _lc3Internal.lookupFunction<
        DecoderGetFrameSamplesC,
        DecoderGetFrameSamplesDart>("decoder_get_frame_samples");
    _isInitialized = true;
  }

  int get encodedDataSize => _encodedDataSize;
  int get decodedDataSize => _decodedDataSize;

  void decoderInit(int sampleRate, int channels, int frameSize, int bitDepth) {
    checkInitialized();
    _assertDecoderResult(_decoderInit(
      sampleRate,
      channels,
      frameSize,
      bitDepth,
    ));
    if (_encodedData != nullptr) {
      calloc.free(_encodedData);
    }
    if (_decodedData != nullptr) {
      calloc.free(_decodedData);
    }
    _encodedDataSize = _decoderGetBlockBytes();
    _encodedData = calloc<Uint8>(_encodedDataSize);
    _decodedDataSize = _decoderGetFrameSamples() * channels * bitDepth ~/ 8;
    _decodedData = calloc<Int16>(_decodedDataSize ~/ 2);
  }

  Uint8List decoderDecode(Iterable<int> encodedData) {
    checkInitialized();
    _encodedData.asTypedList(_encodedDataSize).setAll(0, encodedData);
    _assertDecoderResult(_decoderDecode(_encodedData, _decodedData));
    return _decodedData.asTypedList(_decodedDataSize ~/ 2).buffer.asUint8List();
  }

  void decoderDestroy() {
    checkInitialized();
    _decoderDestroy();
    if (_encodedData != nullptr) {
      calloc.free(_encodedData);
      _encodedData = nullptr;
      _encodedDataSize = 0;
    }
    if (_decodedData != nullptr) {
      calloc.free(_decodedData);
      _decodedData = nullptr;
      _decodedDataSize = 0;
    }
  }

  void _assertDecoderResult(int result) {
    if (result != 0) {
      throw LC3WrapperDecoderError(LC3DecoderError.fromValue(result));
    }
  }
}

abstract class LC3WrapperDecoderException implements Exception {
  final String message;

  LC3WrapperDecoderException(this.message);

  @override
  String toString() {
    return "LC3WrapperDecoderException: $message";
  }
}

class LC3WrapperDecoderNotInitializedException
    extends LC3WrapperDecoderException {
  LC3WrapperDecoderNotInitializedException()
      : super("LC3WrapperDecoder is not initialized");
}

class LC3WrapperDecoderDynamicLibraryOpenException
    extends LC3WrapperDecoderException {
  LC3WrapperDecoderDynamicLibraryOpenException(String path)
      : super("Failed to open dynamic library at path: $path");
}

class LC3WrapperDecoderFunctionLookupException
    extends LC3WrapperDecoderException {
  LC3WrapperDecoderFunctionLookupException(String functionName)
      : super("Failed to lookup function: $functionName");
}

class LC3WrapperDecoderError extends LC3WrapperDecoderException {
  LC3WrapperDecoderError(LC3DecoderError error) : super("Error: $error");
}
