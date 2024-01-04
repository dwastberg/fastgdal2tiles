import unittest
from fastgdal2tiles import _fastgdal2tiles


class AddTestCase(unittest.TestCase):
    def test_add(self):
        r = _fastgdal2tiles.add(1, 2)
        self.assertEqual(r, 3)  # add assertion here


if __name__ == "__main__":
    unittest.main()
