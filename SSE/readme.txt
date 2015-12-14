Работу выполнил студент 324 группы Навражных Вадим Геннадьевич.

Что реализовано:

1) Основная часть в виде дескриптора HOG, grayscale, свёртки с фильтром Собеля, расчёта направления и значения градиента,
   расчёта и нормализации гистограм

Прототипы соответствующих функций из программы:
* void HOG(FMatrix &gs, vector<float> &descryptor, int cell_count);
* FMatrix GreyScale(BMP *img);
* FMatrix HorizontalFilter(FMatrix &img); и FMatrix VerticalFilter(FMatrix &img);
* pair<FMatrix, FMatrix> CalculateVectors(FMatrix &horiz, FMatrix &vert);
* void MakeHist(FMatrix &val, FMatrix &dir, float* histogram); и void NormalizeHist(float *histogram);

Для бинарной классификации на тренеровочной выборке 97% < presision < 99%. 

2) Многоклассовая классификация

С разбиением области направления градиента на 16 частей на тренеровочной выборке precision = 91,5%.
С разбиением области направления градиента на 32 части на тренеровочной выборке  precision = 94%.

3) Нелинейное ядро SVM

Выполнено по алгоритму из описания задания, прототип функции из программы:
void X2Kernel(vector<float> &descryptor);

4) Пирамида дескрипторов

Выполнено по алгоритму из описания задания, прототип функции из программы:
void Pyramide(FMatrix &gs, vector<float> &descryptor);

5) Извлечение цветовых признаков

Выполнено по алгоритму из описания задания, прототип функции из программы:
void ExtractColor(BMP *img, vector<float> &descryptor);



