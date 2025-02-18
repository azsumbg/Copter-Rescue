#pragma once

#include <cstdlib>

constexpr unsigned char CONT_Ok{ 0b00000000 };
constexpr unsigned char CONT_BadPointer{ 0b00000001 };
constexpr unsigned char CONT_BadIndex{ 0b00000010 };
constexpr unsigned char CONT_BadElement{ 0b00000100 };
constexpr unsigned char CONT_BadSize{ 0b00001000 };

namespace cont
{
	template<typename T> class CONTAINER
	{
	private:

		T* mBasePtr{ nullptr };
		size_t max_size{ 0 };
		size_t next_element{ 0 };
		unsigned char last_error{ CONT_Ok };

	public:

		CONTAINER(size_t size = 1) :max_size{ size }
		{
			mBasePtr = reinterpret_cast<T*>(calloc(size, sizeof(T)));
		}
		CONTAINER(T&& other)
		{
			if (mBasePtr != other.mBasePtr)
			{
				max_size = other.size;
				mBasePtr = other.mBasePtr;
				other.mBasePtr = nullptr;
			}
		}
		~CONTAINER()
		{
			if (mBasePtr)free(mBasePtr);
		}

		size_t size() const
		{
			return next_element;
		}
		bool is_valid() const
		{
			return (mBasePtr != nullptr);
		}

		unsigned char push_back(T* element)
		{
			if (mBasePtr == nullptr)return CONT_BadPointer;
			else
			{
				if (next_element + 1 <= max_size)
				{
					mBasePtr[next_element] = *element;
					++next_element;
					return CONT_Ok;
				}
				else
				{
					T* temp_ptr = reinterpret_cast<T*>(realloc(mBasePtr, (max_size + 1) * sizeof(T)));
					mBasePtr = temp_ptr;
					mBasePtr[next_element] = *element;
					++next_element;
					++max_size;
				}
			}

			return CONT_BadElement;
		}
		unsigned char push_back(T&& element)
		{
			if (mBasePtr == nullptr)return CONT_BadPointer;
			else
			{
				if (next_element + 1 <= max_size)
				{
					mBasePtr[next_element] = element;
					++next_element;
					return CONT_Ok;
				}
				else
				{
					T* temp_ptr = reinterpret_cast<T*>(realloc(mBasePtr, (max_size + 1) * sizeof(T)));
					if (temp_ptr)
					{
						mBasePtr = temp_ptr;
						mBasePtr[next_element] = element;
						++next_element;
						++max_size;
						return CONT_Ok;
					}
				}
			}
			return CONT_BadElement;
		}

		unsigned char push_front(T* element)
		{
			if (!mBasePtr)return CONT_BadPointer;

			mBasePtr[0] = *element;

			return CONT_Ok;
		}
		unsigned char push_front(T&& element)
		{
			if (!mBasePtr)return CONT_BadPointer;

			mBasePtr[0] = element;

			return CONT_Ok;
		}

		unsigned char remove(size_t index)
		{
			if (!mBasePtr)return CONT_BadPointer;

			if (index >= max_size || max_size == 1)return CONT_BadIndex;

			T* temp_ptr = reinterpret_cast<T*>(calloc(max_size - 1, sizeof(T)));
			if (temp_ptr)
			{
				for (size_t ind = 0; ind < max_size; ++ind)
				{
					if (ind < index - 1)temp_ptr[ind] = mBasePtr[ind];
					else temp_ptr[ind] = mBasePtr[ind + 1];
				}

				free(mBasePtr);

				mBasePtr = temp_ptr;

				--max_size;
				if (next_element > max_size)next_element = max_size;
			}
			return CONT_Ok;
		}
		unsigned char GetLastError() const
		{
			return last_error;
		}

		T operator[] (size_t index)
		{
			T temp{};

			if (!mBasePtr)
			{
				last_error = CONT_BadPointer;
				return temp;
			}
			if (index >= max_size)
			{
				last_error = CONT_BadIndex;
				return temp;
			}

			return mBasePtr[index];
		}

		unsigned char operator()(size_t index, T* element)
		{
			if (!mBasePtr)return CONT_BadPointer;
			if (index >= max_size)return CONT_BadIndex;

			mBasePtr[index] = *element;

			return CONT_Ok;
		}
		unsigned char operator()(size_t index, T&& element)
		{
			if (!mBasePtr)return CONT_BadPointer;
			if (index >= max_size)return CONT_BadIndex;

			mBasePtr[index] = element;
			return CONT_Ok;
		}
	};
}