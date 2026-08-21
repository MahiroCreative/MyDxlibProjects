#include <iostream>
#include "GameMath.h"
#include <vector>
#include <algorithm>
#include <memory>

using namespace std;

//Vector3のテスト用
void TestVector3();
//vectorのテスト用

int main()
{
	auto y = std::make_shared<int>(1000);

	// 所有権を移動したため、x は何も所有していない。

	std::cout << *y << std::endl;

}

void TestVector3()
{
	/*変数作成*/
	Vector3 ans;
	Vector3 left{ 1,2,3 };
	Vector3 right{ 0.1f,0.2f,0.3f };
	float k = 1.2;
	float fans = 0;

	/*確認*/
	//[+]
	ans = left + right;
	cout << ans.ToString() << std::endl;
	//[+]
	ans = left - right;
	cout << ans.ToString() << std::endl;
	//[+=]
	ans += left;
	cout << ans.ToString() << std::endl;
	//[-=]
	ans -= right;
	cout << ans.ToString() << std::endl;
	//[*]
	ans = left * k;
	cout << ans.ToString() << std::endl;
	//[/]
	ans = left / k;
	cout << ans.ToString() << std::endl;
	//長さ
	cout << left.Length() << endl;
	//内積
	fans = Vector3::Dot(left,right);
	cout << fans << endl;
	//外積
	ans = Vector3::Cross(left,right);
	cout << ans.ToString() << endl;
}