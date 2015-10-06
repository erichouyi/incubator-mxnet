/*!
 *  Copyright (c) 2015 by Contributors
 * \file ndarray.h
 * \brief Rcpp NDArray interface of MXNet
 */
#ifndef MXNET_RCPP_NDARRAY_H_
#define MXNET_RCPP_NDARRAY_H_

#include <Rcpp.h>
#include <mxnet/c_api.h>

namespace mxnet {
namespace R {  // NOLINT(*)

// forward declare NDArrayFunction
class NDArrayFunction;

/*! \brief The Rcpp NDArray class of MXNet */
class NDArray {
 public:
  /*! \brief The type of NDArray in R's side */
  typedef Rcpp::List RObjectType;
  /*! \return the context of the NDArray */
  inline const Context &ctx() const {
    return ctx_;
  }
  /*!
   * \brief create a R object that correspond to the NDArray
   * \param handle the NDArrayHandle needed for output.
   * \param writable Whether the NDArray is writable or not.
   */
  inline static RObjectType RObject(NDArrayHandle handle,
                                    bool writable = true) {
    Rcpp::List ret = Rcpp::List::create(
        Rcpp::Named("ptr") = Rcpp::XPtr<NDArray>(new NDArray(handle, writable)));
    ret.attr("class") = "mx.nd.array";
    return ret;
  }
  /*!
   * \brief Move a existing R NDArray object to a new one.
   * \param src The source R NDArray.
   * \return A new R NDArray containing same information as old one.
   */
  inline static RObjectType Move(const Rcpp::RObject& src) {
    Rcpp::XPtr<NDArray> old = NDArray::XPtr(src);
    old->moved_ = true;
    return NDArray::RObject(old->handle_, old->writable_);
  }
  /*!
   * \brief return extenral pointer representation of NDArray from its R object
   * \param obj The R NDArray object
   * \return The external pointer to the object
   */
  inline static Rcpp::XPtr<NDArray> XPtr(const Rcpp::RObject& obj) {
    Rcpp::List list(obj);
    Rcpp::RObject xptr = list[0];
    Rcpp::XPtr<NDArray> ptr(xptr);
    RCHECK(ptr->moved_)
        << "Passed in a moved NDArray as parameters."
        << " Moved parameters should no longer be used";
    return ptr;
  }
  /*!
   * \brief Load a list of ndarray from the file.
   * \param filename the name of the file.
   * \return R List of NDArrays
   */
  static Rcpp::List Load(const std::string& filename);
  /*!
   * \brief Save a list of NDArray to file.
   * \param data R List of NDArrays
   * \param filename The name of the file to be saved.
   */
  static void Save(const Rcpp::RObject& data,
                   const std::string& filename);
  /*!
   * \brief function to create an empty array
   * \param shape The shape of the Array
   * \return a new created Array
   */
  static RObjectType Empty(const Rcpp::Dimension& shape,
                           const Context::RObjectType& ctx);
  /*! \brief static function to initialize the Rcpp functions */
  static void InitRcppModule();
  /*! \brief destructor */
  ~NDArray() {
    // free the handle
    if (!moved_) {
      MX_CALL(MXNDArrayFree(handle_));
    }
  }

 private:
  /*! \brief default constructor */
  NDArray(NDArrayHandle handle, bool writable)
      : handle_(handle), writable_(writable), moved_(false) {
    MX_CALL(MXNDArrayGetContext(handle,
                                &ctx_.dev_type,
                                &ctx_.dev_id));
  }
  // declare friend class
  friend class NDArrayFunction;
  /*! \brief The context of the NDArray */
  Context ctx_;
  /*! \brief handle to the NDArray */
  NDArrayHandle handle_;
  /*! \brief Whether the NDArray is writable */
  bool writable_;
  /*! \brief Whether this object has been moved to another object */
  bool moved_;
};

/*! \brief The NDArray functions to be invoked */
class NDArrayFunction : public ::Rcpp::CppFunction {
 public:
  virtual SEXP operator() (SEXP * args);

  virtual int nargs() {
    return num_args_;
  }

  virtual bool is_void() {
    return false;
  }

  virtual void signature(std::string& s, const char* name) {
    ::Rcpp::signature< ::Rcpp::void_type >(s, name);
  }

  virtual const char* get_name() {
    return name_.c_str();
  }

  virtual DL_FUNC get_function_ptr() {
    return (DL_FUNC)NULL; // NOLINT(*)
  }
  /*! \brief static function to initialize the Rcpp functions */
  static void InitRcppModule();
 private:
  // make constructor private
  explicit NDArrayFunction(FunctionHandle handle);

  /*! \brief internal functioon handle. */
  FunctionHandle handle_;
  // name of the function
  std::string name_;
  // beginning position of use vars
  mx_uint begin_use_vars_;
  // number of use variable
  mx_uint num_use_vars_;
  // beginning position of scalars
  mx_uint begin_scalars_;
  // number of scalars
  mx_uint num_scalars_;
  // number of mutate variables
  mx_uint num_mutate_vars_;
  // number of arguments
  mx_uint num_args_;
  // whether it accept empty output
  bool accept_empty_out_;
};
}  // namespace Rcpp
}  // namespace mxnet
#endif  // MXNET_RCPP_NDARRAY_H_
