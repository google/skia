function DiffViewController($scope) {
    $scope.differs = [
        {
            "title": "Different Pixels"
        },
        {
            "title": "Perceptual Difference"
        }
    ];
    $scope.sortIndex = 1;
    $scope.records = SkPDiffRecords.records;
    $scope.highlight = function(differName) {
        console.debug(differName);
    }
    $scope.setSortIndex = function(a) {
        $scope.sortIndex = a;
    }
    $scope.sortingDiffer = function(a) {
        console.debug($scope.sortIndex);
        return a.diffs[$scope.sortIndex].result;
    }
}