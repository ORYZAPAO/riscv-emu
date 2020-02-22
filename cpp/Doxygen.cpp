// http://d.hatena.ne.jp/l1o0/20101209/1291903016 より引用させて頂きました．

/**********************/
/* ファイルヘッダの例 */
/**********************/

/******************************************************************************/
/*! @addtogroup モジュール名_サブモジュール名
    @file       comment.c
    @brief      ファイルの概要
*******************************************************************************
    ファイルの詳細な説明
*******************************************************************************
    @date       作成日(YYYY/MM/DD)
    @author     作成者名
    @par        Revision
    $Id$
    @par        Copyright
    20XX-20XX ABC Company Co., Ltd. All rights reserved.
*******************************************************************************
    @par        History
    - 20XX/01/01 更新者名1
      -# Initial Version
    - 20XX/01/02 更新者名2
      -# Add xxx function
      -# Delete yyy function
******************************************************************************/

/*! @ingroup モジュール名_サブモジュール名 */
/* @{ */

#include <stdio.h>

/************/
/* 定義の例 */
/************/

/*! @brief 簡単な定義の説明 */
#define AAA 123

/*! @brief 定義の概要

    定義の詳細
*/
#define BBB 456

#define CCC 758 /*!< 定義の説明を横につける */

/*! @name 定義グループX
    グループ化した定義の説明
*/
/* @{ */
#define DDD 9AB /*!< 定義の説明を横につける1 */
#define EEE CDE /*!< 定義の説明を横につける2 */
/* @} */

/**********************/
/* グローバル変数の例 */
/**********************/

/*! @var   global_var1
    @brief グローバル変数の説明
*/
int global_var1;

/*! @var   global_var2
    @brief グローバル変数の説明

    グローバル変数の詳細
*/
int global_var2;

/**************/
/* 構造体の例 */
/**************/

/*! @struct tStruct1
    @brief  構造体の説明
*/
typedef struct {
    /*! メンバ1の説明 */
    int member1;

    int member2; /*!< メンバ2の説明を横につける */
} tStruct1;

/*! @struct tStruct2
    @brief  構造体の説明

    構造体の詳細な説明
*/
typedef struct {
} tStruct2;

/**************/
/* 列挙型の例 */
/**************/
/*! @enum eEnum1
    @brief  列挙型の説明
*/
typedef enum {
    aaa,

    /*! 列挙型の値の説明 */
    bbb,

    ccc, /*!< 列挙型の値の説明を横につける例 */
} eEnum1;

/*! @enum eEnum2
    @brief  列挙型の説明

    列挙型の詳細な説明
*/
typedef enum {
} eEnum2;

/**************/
/* クラスの例 */
/**************/

/*! @class Class1
    @brief  クラスの説明
*/
class Class1 {
public:
    /*! メンバ1の説明 */
    int member1;

    int member2; /*!< メンバ2の説明を横につける */

    /*! メソッド1の説明 */
    int method1(int var1, int var2);

    /*! メソッド2の説明。詳細説明
        @param[out]     var1    var1の説明
        @param[in]      var2    var2の説明
        @param[in,out]  var3    var3の説明
        @par            Refer
        - 参照するグローバル変数 global_var1
        - 参照するグローバル変数 global_var2
        @par            Modify
        - 変更するグローバル変数 global_var3
        - 変更するグローバル変数 global_var4
        @return         成功 0, 失敗 0 以外 など
        @exception      例外。不要であればnoneを記述
    */
    int method2(int var1, int var2, int var3) {

    }
};

/*! @class Class2
    @brief  クラスの説明

    クラスの詳細
*/
class Class2 {
};

/************/
/* 関数の例 */
/************/

/******************************************************************************/
/*! @brief 関数の概要

    必要であれば詳細な説明
    @param[out]     var1    var1の説明
    @param[in]      var2    var2の説明
    @param[in,out]  var3    var3の説明
    @par            Refer
    - 参照するグローバル変数 global_var1
    - 参照するグローバル変数 global_var2
    @par            Modify
    - 変更するグローバル変数 global_var3
    - 変更するグローバル変数 global_var4
    @return         成功 0, 失敗 0 以外 など
    @exception      例外。不要であればnoneを記述
******************************************************************************/
int func(int var1, char *var2, char *var3[])
{
    return 0;
}

/* @} */
